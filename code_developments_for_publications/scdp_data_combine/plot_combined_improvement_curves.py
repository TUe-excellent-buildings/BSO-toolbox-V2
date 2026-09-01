#!/usr/bin/env python3
"""Combine SCDP improvement curves by run type.

Each output image contains one panel per objective. Thin lines show the
individual runs, while the bold line and shaded band show the median and
interquartile range at each iteration.
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
import textwrap
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch


SCRIPT_DIR = Path(__file__).resolve().parent
REPOSITORY_ROOT = SCRIPT_DIR.parents[1]
sys.path.insert(0, str(REPOSITORY_ROOT))

from bso.utilities.plot.plot_improvement_curves import load_improvement_data


RUN_TYPES = ["sd", "bp", "dl", "sdbpdl", "sdbpdl_weighted"]
RUN_TYPE_LABELS = {
    "sd": "SD",
    "bp": "BP",
    "dl": "DL",
    "sdbpdl": "SDBPDL",
    "sdbpdl_weighted": "SDBPDL weighted (0.333333-0.333333-0.333333)",
}
OBJECTIVE_COLORS = ["#1B6CA8", "#D97706", "#2F855A", "#B83280"]
MAXIMIZATION_OBJECTIVES = {"dl_ADFbuilding"}
COMPACT_RUN_TYPE_LABELS = {
    **RUN_TYPE_LABELS,
    "sdbpdl_weighted": "SDBPDL weighted",
}


@dataclass
class ImprovementRun:
    path: Path
    run_id: str
    run_type: str
    iterations: np.ndarray
    curves: list[np.ndarray]
    names: list[str]
    axis_labels: dict[str, str]


def classify_run_type(run_id: str) -> str | None:
    """Return the optimisation method encoded in a result-directory name."""
    lower_id = run_id.lower()
    weighted_match = re.search(r"_sdbpdl_([^_]+)_-n=", lower_id)
    if weighted_match:
        return "sdbpdl_weighted"
    if "_sdbpdl_-n=" in lower_id:
        return "sdbpdl"
    for run_type in ("sd", "bp", "dl"):
        if f"_{run_type}_-n=" in lower_id:
            return run_type
    return None


def discover_runs(results_dir: Path) -> list[ImprovementRun]:
    runs: list[ImprovementRun] = []
    for path in sorted(results_dir.rglob("improvementCurvesData_*")):
        if not path.is_file():
            continue
        run_type = classify_run_type(path.parent.name)
        if run_type is None:
            print(f"Skipping unrecognised run type: {path}", file=sys.stderr)
            continue
        try:
            iterations, curves, names, _, axis_labels = load_improvement_data(str(path))
        except (OSError, ValueError) as error:
            print(f"Skipping unreadable data file {path}: {error}", file=sys.stderr)
            continue
        if not curves:
            print(f"Skipping data file without objective columns: {path}", file=sys.stderr)
            continue
        runs.append(
            ImprovementRun(
                path=path,
                run_id=path.parent.name,
                run_type=run_type,
                iterations=iterations.astype(int),
                curves=_correct_improvement_signs(curves, names),
                names=names,
                axis_labels=axis_labels,
            )
        )
    return runs


def _objective_label(raw_name: str, axis_labels: dict[str, str]) -> str:
    key = raw_name.removesuffix("_improvement_percent")
    return axis_labels.get(key, key)


def _correct_improvement_signs(curves: list[np.ndarray], names: list[str]) -> list[np.ndarray]:
    corrected: list[np.ndarray] = []
    for curve, raw_name in zip(curves, names[1:]):
        objective_name = raw_name.removesuffix("_improvement_percent")
        if objective_name in MAXIMIZATION_OBJECTIVES:
            corrected.append(-curve)
        else:
            corrected.append(curve)
    return corrected


def _aligned_values(runs: Iterable[ImprovementRun], objective_index: int) -> tuple[np.ndarray, np.ndarray]:
    selected = [run for run in runs if len(run.curves) > objective_index]
    all_iterations = sorted({int(value) for run in selected for value in run.iterations})
    values = np.full((len(selected), len(all_iterations)), np.nan)
    positions = {iteration: index for index, iteration in enumerate(all_iterations)}
    for row, run in enumerate(selected):
        for iteration, value in zip(run.iterations, run.curves[objective_index]):
            values[row, positions[int(iteration)]] = value
    return np.asarray(all_iterations), values


def plot_run_type(run_type: str, runs: list[ImprovementRun], output_path: Path) -> None:
    number_of_objectives = max(len(run.curves) for run in runs)
    columns = 2
    rows = int(np.ceil(number_of_objectives / columns))
    fig, axes = plt.subplots(rows, columns, figsize=(12, 4.5 * rows), squeeze=False)

    for objective_index, axis in enumerate(axes.flat):
        if objective_index >= number_of_objectives:
            axis.set_visible(False)
            continue
        iterations, values = _aligned_values(runs, objective_index)
        color = OBJECTIVE_COLORS[objective_index % len(OBJECTIVE_COLORS)]
        for values_per_run in values:
            axis.plot(iterations, values_per_run, color=color, alpha=0.22, linewidth=1.0)
        median = np.nanmedian(values, axis=0)
        lower = np.nanpercentile(values, 25, axis=0)
        upper = np.nanpercentile(values, 75, axis=0)
        axis.fill_between(iterations, lower, upper, color=color, alpha=0.20, label="IQR")
        axis.plot(iterations, median, color=color, linewidth=2.5, label="Median")
        axis.set_title(_objective_label(runs[0].names[objective_index + 1], runs[0].axis_labels))
        axis.set_xlabel("Iteration")
        axis.set_ylabel("Improvement vs initial design [%]")
        axis.set_xlim(0, 10)
        axis.set_xticks(range(0, 11))
        if runs[0].names[objective_index + 1].startswith("sd_"):
            axis.set_ylim(bottom=-500)
            axis.set_ylim(top=100)
        axis.grid(True, linestyle=":", linewidth=0.6, alpha=0.8)
        axis.legend(loc="best")

    fig.suptitle(
        f"Improvement curves: {RUN_TYPE_LABELS[run_type]} ({len(runs)} runs)",
        fontsize=14,
    )
    fig.tight_layout(rect=(0, 0, 1, 0.96))
    fig.savefig(output_path, dpi=220)
    plt.close(fig)


def write_combined_page(
    runs: list[ImprovementRun],
    output_path: Path,
    page_width_mm: float = 170.0,
    page_height_mm: float = 150.0,
) -> None:
    """Render one compact page with run types as rows and objectives as columns."""
    if not runs:
        print("No runs found to build the summary page.", file=sys.stderr)
        return

    run_types = [run_type for run_type in RUN_TYPES if any(run.run_type == run_type for run in runs)]
    number_of_objectives = max(len(run.curves) for run in runs)
    rows = len(run_types)
    columns = number_of_objectives
    mm_to_inch = 1.0 / 25.4
    # Change rows * 40.0 to adjust panel height; page_height_mm is the maximum.
    height_mm = min(page_height_mm, max(45.0, rows * 40.0))
    fig, axes = plt.subplots(
        rows,
        columns,
        figsize=(page_width_mm * mm_to_inch, height_mm * mm_to_inch),
        squeeze=False,
    )

    for row, run_type in enumerate(run_types):
        group = [run for run in runs if run.run_type == run_type]
        for objective_index in range(columns):
            axis = axes[row, objective_index]
            if objective_index >= len(group[0].curves):
                axis.set_visible(False)
                continue
            iterations, values = _aligned_values(group, objective_index)
            color = OBJECTIVE_COLORS[objective_index % len(OBJECTIVE_COLORS)]
            for values_per_run in values:
                axis.plot(iterations, values_per_run, color=color, alpha=0.22, linewidth=0.7)
            axis.fill_between(
                iterations,
                np.nanpercentile(values, 25, axis=0), # set the value range for coloring the interquartile range (IQR), i.e., between the 25th and 75th percentiles of the values for each iteration
                np.nanpercentile(values, 75, axis=0), # set the value range for coloring the interquartile range (IQR), i.e., between the 25th and 75th percentiles of the values for each iteration
                color=color,
                alpha=0.20,
            )
            axis.plot(iterations, np.nanmedian(values, axis=0), color=color, linewidth=1.5) # set the graph line widths
            axis.set_xlim(0, 10)
            axis.set_xticks(range(0, 11, 2))
            finite_values = values[np.isfinite(values)]
            if finite_values.size and np.min(finite_values) < -500:
                axis.set_ylim(bottom=-500)
                if group[0].names[objective_index + 1].startswith("sd_"):
                    axis.set_ylim(top=100)
            axis.grid(True, linestyle=":", linewidth=0.45, alpha=0.8)
            axis.tick_params(labelsize=8, pad=1)
            if row == 0:
                axis.set_title(
                    textwrap.fill(
                        _objective_label(
                            group[0].names[objective_index + 1], group[0].axis_labels
                        ),
                        width=21,
                    ),
                        fontsize=8,
                        pad=3,  # Gap between objective titles and the graphs.
                )
            if row == rows - 1:
                axis.set_xlabel("Iteration", fontsize=8, labelpad=2)  # Gap below bottom graphs.

    # Main layout controls: left/right/bottom/top are page margins; wspace/hspace
    # control horizontal/vertical spacing between graph panels.
    fig.subplots_adjust(left=0.10, right=0.985, bottom=0.06, top=0.92, wspace=0.35, hspace=0.20)
    for row, run_type in enumerate(run_types):
        row_axis = axes[row, 0]
        row_position = row_axis.get_position()
        row_center = row_position.y0 + row_position.height / 2
        fig.text(
            0.010,  # Horizontal position of the row labels at the page edge.
            row_center,
            COMPACT_RUN_TYPE_LABELS[run_type],
            ha="center",
            va="center",
            rotation=90,
            fontsize=8,
        )
        fig.text(
            0.032,  # Horizontal position of the per-row Improvement [%] labels.
            row_center,
            "Improvement [%]",
            ha="center",
            va="center",
            rotation=90,
            fontsize=8,
        )

    legend_handles = [
        Line2D([0], [0], color="#333333", linewidth=1.5, label="Median"),
        Patch(facecolor="#777777", alpha=0.20, edgecolor="none", label="interquartile range (IQR)"),
    ]
    fig.legend(
        handles=legend_handles,
        loc="upper right",
        bbox_to_anchor=(0.995, 1.0),  # Legend position: x, y in figure coordinates.
        ncol=2,
        frameon=False,
        fontsize=8,
        handlelength=2,
    )
    #fig.suptitle(
        #"Improvement curves by run type",
       # fontsize=8,
       # y=0.995,  # Vertical position of the page title in figure coordinates.
       # x=0.24,  # Horizontal position of the page title in figure coordinates.
    #)
    fig.savefig(output_path, dpi=300)
    plt.close(fig)


def write_manifest(runs: Iterable[ImprovementRun], output_path: Path) -> None:
    with output_path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["run_type", "run_id", "data_file"])
        for run in runs:
            writer.writerow([run.run_type, run.run_id, run.path])


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Combine improvement curve plots by SCDP run type."
    )
    parser.add_argument(
        "results_dir",
        nargs="?",
        type=Path,
        default=Path("/home/tessa/Data/Results_Modify_20/results"),
        help="Results directory to scan (default: /home/tessa/Data/Results_Modify_20/results).",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=None,
        help="Directory for combined plots (default: <results_dir>/combined_improvement_curves).",
    )
    args = parser.parse_args()

    if not args.results_dir.is_dir():
        parser.error(f"Results directory does not exist: {args.results_dir}")
    output_dir = args.output_dir or args.results_dir / "combined_improvement_curves"
    output_dir.mkdir(parents=True, exist_ok=True)

    runs = discover_runs(args.results_dir)
    if not runs:
        parser.error("No improvementCurvesData_* files could be loaded.")

    write_manifest(runs, output_dir / "combined_improvement_curves_manifest.csv")
    generated_paths: list[Path] = []
    for run_type in RUN_TYPES:
        group = [run for run in runs if run.run_type == run_type]
        if not group:
            continue
        output_path = output_dir / f"combined_improvement_curves_{run_type}.png"
        plot_run_type(run_type, group, output_path)
        generated_paths.append(output_path)
        print(f"Wrote {output_path} ({len(group)} runs)")

    summary_page_path = output_dir / "combined_improvement_curves_summary_page.png"
    write_combined_page(runs, summary_page_path)
    print(f"Wrote {summary_page_path}")


if __name__ == "__main__":
    main()