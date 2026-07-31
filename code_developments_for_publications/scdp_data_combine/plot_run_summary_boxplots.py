#!/usr/bin/env python3
"""Aggregate summary variables from run CSV files and plot boxplots.

This script scans one or more root directories recursively for CSV files,
excluding files named exactly combined_data.csv. For each CSV, it extracts
summary rows like:
  - sdBest iteration
  - bpBest iteration
  - dl_ADFbuilding_inverseBest iteration
  - dl_scoreBest iteration
  - sd last iteration vs. first iteration
  - ...
  - sd best iteration vs. first iteration
  - ...

Outputs:
  - summary_variable_values_long.csv
    - boxplot_best_iteration.png
    - boxplot_last_vs_first.png
    - boxplot_best_vs_first.png
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import re
import textwrap
from typing import Dict, Iterable, List, Optional

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


DEFAULT_METRICS = ["sd", "bp", "dl_ADFbuilding_inverse", "dl_score"]

RUN_TYPE_ORDER = ["sd", "bp", "dl", "sdbpdl", "sdbpdl_weighted"]

RUN_TYPE_DISPLAY = {
    "sd": "Structural",
    "bp": "Thermal",
    "dl": "Daylight",
    "sdbpdl": "Sequential",
    "sdbpdl_weighted": "Weighted",
}


def _safe_float(value: str) -> Optional[float]:
    try:
        return float(value.strip())
    except Exception:
        return None


def _extract_dl_score_first_and_last(rows: List[List[str]]) -> tuple[Optional[float], Optional[float]]:
    dl_score_col: Optional[int] = None
    iteration_to_dl_score: Dict[int, float] = {}

    for r in rows:
        # Header row for the main iteration table, e.g. ['', 'sd', 'bp', 'dl_ADFbuilding_inverse', 'dl_score', ...]
        if dl_score_col is None and len(r) >= 2:
            normalized = [c.strip() for c in r]
            if "sd" in normalized and "dl_score" in normalized:
                try:
                    dl_score_col = normalized.index("dl_score")
                except ValueError:
                    dl_score_col = None
                continue

        if dl_score_col is None:
            continue
        if len(r) <= dl_score_col:
            continue

        iter_token = r[0].strip() if r else ""
        if not iter_token.isdigit():
            continue

        dl_val = _safe_float(r[dl_score_col])
        if dl_val is None or not np.isfinite(dl_val):
            continue

        iteration_to_dl_score[int(iter_token)] = float(dl_val)

    if not iteration_to_dl_score:
        return None, None

    first = iteration_to_dl_score.get(0)
    if first is None:
        first = iteration_to_dl_score[min(iteration_to_dl_score.keys())]
    last = iteration_to_dl_score[max(iteration_to_dl_score.keys())]
    return first, last


def build_target_labels(metrics: List[str]) -> List[str]:
    labels: List[str] = []
    for m in metrics:
        labels.append(f"{m}Best iteration")
    for m in metrics:
        labels.append(f"{m} last iteration vs. first iteration")
    for m in metrics:
        labels.append(f"{m} best iteration vs. first iteration")
    return labels


def select_labels_by_group(all_labels: List[str], group_key: str) -> List[str]:
    if group_key == "best_iteration":
        return [lbl for lbl in all_labels if lbl.endswith("Best iteration")]
    if group_key == "last_vs_first":
        return [lbl for lbl in all_labels if "last iteration vs. first iteration" in lbl]
    if group_key == "best_vs_first":
        return [lbl for lbl in all_labels if "best iteration vs. first iteration" in lbl]
    raise ValueError(f"Unknown group_key: {group_key}")


def discover_csv_files(input_roots: Iterable[Path]) -> List[Path]:
    files: List[Path] = []
    for root in input_roots:
        for p in sorted(root.rglob("*.csv")):
            if p.stem == "combined_data":
                continue
            files.append(p)
    return files


def classify_run_type(run_id: str) -> str:
    rid = run_id.strip().lower()
    if "_sdbpdl_" in rid:
        tail = rid.split("_sdbpdl_", 1)[1]
        # Expected examples:
        #   design_x_sdbpdl_-n=10_...
        #   design_x_sdbpdl_0.3-0.3-0.4_-n=10_...
        weight_match = re.search(r"^(.*?)_-n=", tail)
        if weight_match:
            weight_part = weight_match.group(1).strip()
            if weight_part:
                return "sdbpdl_weighted"
            return "sdbpdl"
        first_token = tail.split("_", 1)[0].strip()
        if first_token.startswith("-n="):
            return "sdbpdl"
        if first_token and "-" in first_token:
            return "sdbpdl_weighted"
        return "sdbpdl"
    if "_sd_" in rid:
        return "sd"
    if "_bp_" in rid:
        return "bp"
    if "_dl_" in rid:
        return "dl"
    return "unknown"


def collect_summary_values(csv_files: Iterable[Path], target_labels: List[str]) -> pd.DataFrame:
    target_set = set(target_labels)
    rows: List[Dict[str, object]] = []

    for file_path in csv_files:
        run_id = file_path.parent.name
        run_type = classify_run_type(run_id)
        with file_path.open("r", newline="") as f:
            reader = csv.reader(f)
            file_rows = list(reader)
            dl_score_first: Optional[float] = None
            dl_score_last: Optional[float] = None
            has_extracted_dl_score_edges = False

            for r in file_rows:
                if len(r) < 2:
                    continue
                label = r[0].strip()
                if label not in target_set:
                    continue

                raw_value = r[1].strip()
                val = _safe_float(raw_value)
                if val is None:
                    continue

                if np.isnan(val):
                    if label == "dl_score best iteration vs. first iteration":
                        val = 0.0
                    elif label == "dl_score last iteration vs. first iteration":
                        if not has_extracted_dl_score_edges:
                            dl_score_first, dl_score_last = _extract_dl_score_first_and_last(file_rows)
                            has_extracted_dl_score_edges = True
                        if (
                            dl_score_first is not None
                            and dl_score_last is not None
                            and np.isclose(dl_score_first, 0.0)
                            and np.isclose(dl_score_last, 0.0)
                        ):
                            val = 0.0
                        else:
                            # Keep as missing when ratio is undefined but not the 0/0 case at last iteration.
                            continue
                    else:
                        continue

                if not np.isfinite(val):
                    continue
                rows.append(
                    {
                        "file_path": str(file_path),
                        "run_id": run_id,
                        "run_type": run_type,
                        "variable": label,
                        "value": float(val),
                    }
                )

    return pd.DataFrame.from_records(rows)


def plot_boxplot_all_variables(
    long_df: pd.DataFrame,
    ordered_labels: List[str],
    output_path: Path,
    title: str,
    y_label: str = "Value",
    x_label_overrides: Optional[Dict[str, str]] = None,
    y_min: Optional[float] = None,
    y_max: Optional[float] = None,
    x_label_wrap_width: int = 16,
) -> None:
    non_empty_labels = [lbl for lbl in ordered_labels if (long_df["variable"] == lbl).any()]
    if not non_empty_labels:
        raise ValueError("No target summary variables found to plot.")

    groups = [
        long_df.loc[long_df["variable"] == lbl, "value"].to_numpy(dtype=float)
        for lbl in non_empty_labels
    ]

    display_labels = [x_label_overrides.get(lbl, lbl) for lbl in non_empty_labels] if x_label_overrides else non_empty_labels
    wrapped_labels = [textwrap.fill(lbl, width=x_label_wrap_width, break_long_words=False) for lbl in display_labels]

    fig, ax = plt.subplots(figsize=(85/25.4, 60/25.4)) # /25.4 to convert from mm to inches
    bp = ax.boxplot(
        groups,
        labels=wrapped_labels,
        patch_artist=True,
        showfliers=False,
        boxprops=dict(linewidth=0.6),
        whiskerprops=dict(linewidth=0.6),
        capprops=dict(linewidth=0.6),
        medianprops=dict(linewidth=0.6),
        flierprops=dict(linewidth=0.6),
    )

    palette = ["#2E7D32", "#1565C0", "#C62828", "#6A1B9A"]
    for i, patch in enumerate(bp["boxes"]):
        patch.set_facecolor(palette[i % len(palette)])
        patch.set_alpha(0.42)

    for median in bp["medians"]:
        median.set_color("#202020")

    ax.axhline(0.0, color="#666666", linewidth=0.6, linestyle="--")
    ax.set_ylabel(y_label)
    ax.set_title(title)
    ax.grid(axis="y", alpha=0.2)
    if y_min is not None or y_max is not None:
        ax.set_ylim(bottom=y_min, top=y_max)

    fig.tight_layout()
    fig.savefig(output_path, dpi=260)
    plt.close(fig)


def plot_grouped_boxplot_on_axis(
    long_df: pd.DataFrame,
    run_types: List[str],
    group_labels: List[str],
    ax: plt.Axes,
    title: str,
    y_label: str,
    x_label_overrides: Optional[Dict[str, str]] = None,
    y_min: Optional[float] = None,
    y_max: Optional[float] = None,
    x_label_wrap_width: int = 16,
    show_x_labels: bool = True,
) -> tuple[List[object], List[str]]:
    if not run_types:
        raise ValueError("No run types available to plot.")

    palette = ["#2E7D32", "#1565C0", "#C62828", "#6A1B9A"]
    n_metrics = len(group_labels)
    group_gap = 1.2
    box_width = 0.7
    section_width = n_metrics + group_gap
    group_centers = [idx * section_width + (n_metrics - 1) / 2 for idx in range(len(run_types))]
    section_bounds = [
        (center - section_width / 2, center + section_width / 2)
        for center in group_centers
    ]

    values_list: List[np.ndarray] = []
    positions: List[float] = []
    metric_indices: List[int] = []

    for rt_idx, run_type in enumerate(run_types):
        base = rt_idx * (n_metrics + group_gap)
        for metric_idx, metric_label in enumerate(group_labels):
            vals = long_df.loc[
                (long_df["run_type"] == run_type) & (long_df["variable"] == metric_label),
                "value",
            ].to_numpy(dtype=float)
            if vals.size == 0:
                continue
            values_list.append(vals)
            positions.append(base + metric_idx)
            metric_indices.append(metric_idx)

    if not values_list:
        raise ValueError("No matching data found for grouped run-type plot.")

    bp = ax.boxplot(
        values_list,
        positions=positions,
        widths=box_width,
        patch_artist=True,
        showfliers=False,
        boxprops=dict(linewidth=0.6),
        whiskerprops=dict(linewidth=0.6),
        capprops=dict(linewidth=0.6),
        medianprops=dict(linewidth=0.6),
        flierprops=dict(linewidth=0.6),
    )

    legend_handles: Dict[int, object] = {}
    for i, patch in enumerate(bp["boxes"]):
        color = palette[metric_indices[i] % len(palette)]
        patch.set_facecolor(color)
        patch.set_alpha(0.42)
        if metric_indices[i] not in legend_handles:
            legend_handles[metric_indices[i]] = patch

    for median in bp["medians"]:
        median.set_color("#202020")

    # Visually mark each run-type area with equal-width alternating bands.
    for idx, (left, right) in enumerate(section_bounds):
        if idx % 2 == 0:
            ax.axvspan(left, right, facecolor="#000000", alpha=0.03, zorder=0)
        if idx < len(section_bounds) - 1:
            ax.axvline(right, color="#BBBBBB", linewidth=0.4, linestyle=":", zorder=0)

    if section_bounds:
        ax.set_xlim(section_bounds[0][0], section_bounds[-1][1])
    ax.margins(x=0.0)

    group_names = [textwrap.fill(RUN_TYPE_DISPLAY.get(rt, rt), width=x_label_wrap_width, break_long_words=False) for rt in run_types]
    ax.set_xticks(group_centers)
    ax.set_xticklabels(group_names if show_x_labels else [""] * len(group_names))
    ax.tick_params(axis="x", length=0)

    metric_display = [x_label_overrides.get(lbl, lbl) for lbl in group_labels] if x_label_overrides else group_labels
    metric_display = [textwrap.fill(lbl, width=22, break_long_words=False) for lbl in metric_display]
    handles = [legend_handles[idx] for idx in range(n_metrics) if idx in legend_handles]
    labels = [metric_display[idx] for idx in range(n_metrics) if idx in legend_handles]

    ax.axhline(0.0, color="#666666", linewidth=0.6, linestyle="--")
    ax.set_xlabel("", labelpad=8)
    ax.set_ylabel(y_label)
    ax.set_title(title)
    ax.grid(axis="y", alpha=0.2)
    if y_min is not None or y_max is not None:
        ax.set_ylim(bottom=y_min, top=y_max)
    return handles, labels


def plot_boxplot_grouped_by_run_type(
    long_df: pd.DataFrame,
    run_types: List[str],
    group_labels: List[str],
    output_path: Path,
    title: str,
    y_label: str,
    x_label_overrides: Optional[Dict[str, str]] = None,
    y_min: Optional[float] = None,
    y_max: Optional[float] = None,
    x_label_wrap_width: int = 16,
) -> None:
    fig, ax = plt.subplots(figsize=(170 / 25.4, 72 / 25.4))
    handles, labels = plot_grouped_boxplot_on_axis(
        long_df,
        run_types,
        group_labels,
        ax,
        title,
        y_label,
        x_label_overrides=x_label_overrides,
        y_min=y_min,
        y_max=y_max,
        x_label_wrap_width=x_label_wrap_width,
        show_x_labels=True,
    )
    if handles:
        ax.legend(
            handles,
            labels,
            loc="upper left",
            bbox_to_anchor=(0.0, -0.30, 1.0, 0.2),
            ncol=min(4, len(handles)),
            frameon=False,
            mode="expand",
            borderaxespad=0.0,
        )
    fig.tight_layout(rect=(0.0, 0.16, 1.0, 1.0))
    fig.savefig(output_path, dpi=260, bbox_inches="tight", pad_inches=0.02)
    plt.close(fig)


def plot_boxplot_grouped_runtypes_three_in_one(
    long_df: pd.DataFrame,
    run_types: List[str],
    best_iteration_labels: List[str],
    last_vs_first_labels: List[str],
    best_vs_first_labels: List[str],
    output_path: Path,
    x_label_overrides: Optional[Dict[str, str]] = None,
) -> None:
    fig, axes = plt.subplots(3, 1, figsize=(170 / 25.4, 190 / 25.4))

    handles, labels = plot_grouped_boxplot_on_axis(
        long_df,
        run_types,
        best_vs_first_labels,
        axes[0],
        "Percentage of improvement best vs. first iteration",
        "Objective improvement [%]",
        x_label_overrides=x_label_overrides,
        y_min=0.0,
        y_max=100.0,
        show_x_labels=True,
    )
    plot_grouped_boxplot_on_axis(
        long_df,
        run_types,
        last_vs_first_labels,
        axes[1],
        "Percentage of improvement Last-vs-First iteration",
        "Objective improvement [%]",
        x_label_overrides=x_label_overrides,
        y_max=100.0,
        y_min=-500.0,
        show_x_labels=True,
    )
    plot_grouped_boxplot_on_axis(
        long_df,
        run_types,
        best_iteration_labels,
        axes[2],
        "Best iteration",
        "Best iteration for its objective",
        x_label_overrides=x_label_overrides,
        y_min=0.0,
        y_max=10.0,
        show_x_labels=True,
    )

    if handles:
        axes[2].legend(
            handles,
            labels,
            loc="upper left",
            bbox_to_anchor=(0.0, -0.30, 1.0, 0.2),
            ncol=min(4, len(handles)),
            frameon=False,
            mode="expand",
            borderaxespad=0.0,
        )
    fig.tight_layout(rect=(0.0, 0.10, 1.0, 1.0))
    fig.savefig(output_path, dpi=260, bbox_inches="tight", pad_inches=0.02)
    plt.close(fig)


def main() -> None:
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["cmss10", "Computer Modern Sans Serif", "DejaVu Sans"],
        "font.size": 8,
        "axes.titlesize": 8,
        "axes.labelsize": 8,
        "xtick.labelsize": 8,
        "ytick.labelsize": 8,
        "axes.unicode_minus": False,
        "axes.linewidth": 0.6,
        "xtick.major.width": 0.6,
        "ytick.major.width": 0.6,
    })

    parser = argparse.ArgumentParser(
        description="Create boxplots for summary variables extracted from run CSV files."
    )
    parser.add_argument(
        "input_roots",
        nargs="+",
        type=Path,
        help="One or more root directories to search recursively for CSV files.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("summary_boxplot_output"),
        help="Output directory for generated CSV and PNG files.",
    )
    parser.add_argument(
        "--metrics",
        nargs="+",
        default=DEFAULT_METRICS,
        help="Metric prefixes used to build target summary variable names.",
    )

    args = parser.parse_args()

    input_roots: List[Path] = args.input_roots
    output_dir: Path = args.output_dir
    metrics: List[str] = args.metrics

    output_dir.mkdir(parents=True, exist_ok=True)

    ordered_labels = build_target_labels(metrics)
    csv_files = discover_csv_files(input_roots)
    long_df = collect_summary_values(csv_files, ordered_labels)
    if long_df.empty:
        raise SystemExit(
            "No matching summary rows were found. Check input paths, metrics, and file contents."
        )

    long_csv_path = output_dir / "summary_variable_values_long.csv"
    best_iteration_plot_path = output_dir / "boxplot_best_iteration.png"
    last_vs_first_plot_path = output_dir / "boxplot_last_vs_first.png"
    best_vs_first_plot_path = output_dir / "boxplot_best_vs_first.png"
    grouped_best_iteration_plot_path = output_dir / "boxplot_grouped_runtypes_best_iteration.png"
    grouped_last_vs_first_plot_path = output_dir / "boxplot_grouped_runtypes_last_vs_first.png"
    grouped_best_vs_first_plot_path = output_dir / "boxplot_grouped_runtypes_best_vs_first.png"
    grouped_three_in_one_plot_path = output_dir / "boxplot_grouped_runtypes_all_three.png"

    long_df.to_csv(long_csv_path, index=False)

    best_iteration_labels = select_labels_by_group(ordered_labels, "best_iteration")
    last_vs_first_labels = select_labels_by_group(ordered_labels, "last_vs_first")
    best_vs_first_labels = select_labels_by_group(ordered_labels, "best_vs_first")

    _display_names = ["Strain energy [Nmm]", "Heating and cooling energy [kWh]", "Inverse average daylight factor[1%]", "BS EN 17037-2018 section B.3.2 score [-]"]
    y_label_overrides = {}
    for m, name in zip(metrics, _display_names):
        y_label_overrides[f"{m}Best iteration"] = name
        y_label_overrides[f"{m} last iteration vs. first iteration"] = name
        y_label_overrides[f"{m} best iteration vs. first iteration"] = name

    plot_boxplot_all_variables(
        long_df,
        best_iteration_labels,
        best_iteration_plot_path,
        "Distribution of Best Iteration for all scdp runs",
        y_label="Best iteration for its objective",
        x_label_overrides=y_label_overrides,
        y_min=0.0,
        y_max=10.0,
    )
    plot_boxplot_all_variables(
        long_df,
        last_vs_first_labels,
        last_vs_first_plot_path,
        "Distribution of Last-vs-First iteration for all objectives",
        y_label="Objective improvement [%]",
        x_label_overrides=y_label_overrides,
        y_max=100.0
    )
    plot_boxplot_all_variables(
        long_df,
        best_vs_first_labels,
        best_vs_first_plot_path,
        "Distribution of best-vs-first iteration for all objectives",
        y_label="Objective improvement [%]",
        x_label_overrides=y_label_overrides,
        y_min=0.0,
        y_max=100.0
    )

    run_types_present = [rt for rt in RUN_TYPE_ORDER if (long_df["run_type"] == rt).any()]

    plot_boxplot_grouped_by_run_type(
        long_df,
        run_types_present,
        best_iteration_labels,
        grouped_best_iteration_plot_path,
        "Best iteration ",
        y_label="Best iteration for its objective",
        x_label_overrides=y_label_overrides,
        y_min=0.0,
        y_max=10.0,
    )
    plot_boxplot_grouped_by_run_type(
        long_df,
        run_types_present,
        last_vs_first_labels,
        grouped_last_vs_first_plot_path,
        "Last-vs-First ",
        y_label="Objective improvement [%]",
        x_label_overrides=y_label_overrides,
        y_max=100.0,
        y_min=-500
    )
    plot_boxplot_grouped_by_run_type(
        long_df,
        run_types_present,
        best_vs_first_labels,
        grouped_best_vs_first_plot_path,
        "Improvement best vs. first iteration",
        y_label="Objective improvement [%]",
        x_label_overrides=y_label_overrides,
        y_min=-100.0,
        y_max=200.0,
    )
    plot_boxplot_grouped_runtypes_three_in_one(
        long_df,
        run_types_present,
        best_iteration_labels,
        last_vs_first_labels,
        best_vs_first_labels,
        grouped_three_in_one_plot_path,
        x_label_overrides=y_label_overrides,
    )

    for run_type in run_types_present:
        run_df = long_df.loc[long_df["run_type"] == run_type]
        run_display = RUN_TYPE_DISPLAY.get(run_type, run_type)

        run_best_iteration_plot_path = output_dir / f"boxplot_best_iteration_{run_type}.png"
        run_last_vs_first_plot_path = output_dir / f"boxplot_last_vs_first_{run_type}.png"
        run_best_vs_first_plot_path = output_dir / f"boxplot_best_vs_first_{run_type}.png"

        plot_boxplot_all_variables(
            run_df,
            best_iteration_labels,
            run_best_iteration_plot_path,
            f"Distribution of Best iteration for {run_display} runs",
            y_label="Best iteration for its objective",
            x_label_overrides=y_label_overrides,
            y_min=0.0,
            y_max=10.0,
        )
        plot_boxplot_all_variables(
            run_df,
            last_vs_first_labels,
            run_last_vs_first_plot_path,
            f"Distribution of Last-vs-First iteration for {run_display} runs",
            y_label="Objective improvement [%]",
            x_label_overrides=y_label_overrides,
            y_max=100.0,
        )
        plot_boxplot_all_variables(
            run_df,
            best_vs_first_labels,
            run_best_vs_first_plot_path,
            f"Distribution of best vs. first iteration for {run_display} runs",
            y_label="Objective improvement [%]",
            x_label_overrides=y_label_overrides,
            y_min=0.0,
            y_max=100.0,
        )

    print("Scanned CSV files:", len(csv_files))
    print("Matched rows:", len(long_df))
    print("Outputs:")
    print(" -", long_csv_path)
    print(" -", best_iteration_plot_path)
    print(" -", last_vs_first_plot_path)
    print(" -", best_vs_first_plot_path)
    print("Grouped all-run-type outputs:")
    print(" -", grouped_best_iteration_plot_path)
    print(" -", grouped_last_vs_first_plot_path)
    print(" -", grouped_best_vs_first_plot_path)
    print(" -", grouped_three_in_one_plot_path)
    if run_types_present:
        print("Per-run-type outputs:")
        for run_type in run_types_present:
            print(" -", output_dir / f"boxplot_best_iteration_{run_type}.png")
            print(" -", output_dir / f"boxplot_last_vs_first_{run_type}.png")
            print(" -", output_dir / f"boxplot_best_vs_first_{run_type}.png")


if __name__ == "__main__":
    main()
