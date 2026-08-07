#!/usr/bin/env python3
"""Create 3D objective plots comparing modify vs split/delete runs.

For each matched design key found in both roots, this script reads:
  <run_dir>/combined_data_parallel.csv

It extracts the iteration table rows (0, 1, 2, ...) for the columns:
  sd, bp, dl_ADFbuilding

Then it generates one 3D scatter plot per design key and optional combined plots.
Coloring can be by:
    - modification method (modify vs split/delete), or
    - iteration number (yellow -> red).

Optional Pareto-front lines can be overlaid.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
import re
from typing import Dict, Iterable, List, Optional, Tuple

import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
from matplotlib.lines import Line2D
import numpy as np


AXIS_LABELS = {
    "sd": "Total strain energy [Nmm]",
    "bp": "Heating and cooling energy [kWh]",
    "dl_ADFbuilding": "Average daylight factor [%]",
}

DEFAULT_MODIFY_ROOT = Path("/home/tessa/Data/Results_modify/results")
DEFAULT_SPLIT_DELETE_ROOT = Path("/home/tessa/Data/Results_split_delete/results")
OBJECTIVE_ORDER = ["sd", "bp", "dl", "sdbpdl", "sdbpdl_weighted"]
OBJECTIVE_LABELS = {
    "sd": "sd",
    "bp": "bp",
    "dl": "dl",
    "sdbpdl": "sdbpdl",
    "sdbpdl_weighted": "sdbpdl_*",
}
OBJECTIVE_MARKERS = {
    "sd": "o",
    "bp": "^",
    "dl": "s",
    "sdbpdl": "D",
    "sdbpdl_weighted": "P",
}


@dataclass
class RunData:
    design_key: str
    run_dir: Path
    csv_path: Path
    iterations: np.ndarray
    sd: np.ndarray
    bp: np.ndarray
    dl_adf_inv: np.ndarray


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Generate one 3D objective scatter plot per design, comparing "
            "modify vs split/delete runs from combined_data_parallel.csv files."
        )
    )
    parser.add_argument(
        "--modify-root",
        type=Path,
        default=DEFAULT_MODIFY_ROOT,
        help=f"Root directory for modify runs (default: {DEFAULT_MODIFY_ROOT})",
    )
    parser.add_argument(
        "--split-delete-root",
        type=Path,
        default=DEFAULT_SPLIT_DELETE_ROOT,
        help=(
            "Root directory for split/delete runs "
            f"(default: {DEFAULT_SPLIT_DELETE_ROOT})"
        ),
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("objective3d_compare_plots"),
        help="Directory where output PNGs are written.",
    )
    parser.add_argument(
        "--csv-name",
        type=str,
        default="combined_data_parallel.csv",
        help="CSV file name expected in each run directory.",
    )
    parser.add_argument(
        "--design-filter",
        type=str,
        default="",
        help=(
            "Optional substring filter for design keys, "
            "e.g. 'design_1_bp' to plot only matching designs."
        ),
    )
    parser.add_argument(
        "--base-designs",
        nargs="+",
        default=[],
        help=(
            "Optional base design names to include only specific designs, "
            "e.g. --base-designs design_1 design_2 design_3"
        ),
    )
    parser.add_argument(
        "--combine-selected-designs",
        action="store_true",
        help=(
            "Also generate one extra plot that combines all selected design keys "
            "from both methods into a single figure."
        ),
    )
    parser.add_argument(
        "--color-mode",
        choices=["method", "iteration"],
        default="method",
        help=(
            "Point coloring mode: 'method' uses one color per method; "
            "'iteration' colors points by iteration number (yellow -> red)."
        ),
    )
    parser.add_argument(
        "--show-pareto-front",
        action="store_true",
        help="Overlay a Pareto-front line for each plotted run.",
    )
    parser.add_argument(
        "--dpi",
        type=int,
        default=150,
        help="PNG resolution.",
    )
    parser.add_argument(
        "--interactive",
        action="store_true",
        help=(
            "Open interactive plot windows so you can rotate/zoom/pan manually "
            "before each figure is saved."
        ),
    )
    parser.add_argument(
        "--skip-combined-objectives",
        action="store_true",
        help="Skip aggregated plots that combine sd/bp/dl/sdbpdl/sdbpdl_* per base design.",
    )
    return parser.parse_args()


def design_key_from_run_dir_name(name: str) -> str:
    """Extract design key from a run dir name.

    Example:
      design_1_bp_-n=10_2026-04-25_11-12-29 -> design_1_bp
    """
    marker = "_-n="
    idx = name.find(marker)
    if idx == -1:
        return name
    return name[:idx]


def collect_latest_run_dirs(root: Path, csv_name: str) -> Dict[str, Path]:
    """Map design_key -> latest run directory under root.

    If multiple directories share the same design key, the lexicographically
    largest directory name is used (works with YYYY-MM-DD_HH-MM-SS suffixes).
    """
    selected: Dict[str, Path] = {}

    if not root.is_dir():
        raise FileNotFoundError(f"Root directory not found: {root}")

    for child in sorted(root.iterdir()):
        if not child.is_dir():
            continue
        csv_path = child / csv_name
        if not csv_path.is_file():
            continue

        key = design_key_from_run_dir_name(child.name)
        prev = selected.get(key)
        if prev is None or child.name > prev.name:
            selected[key] = child

    return selected


def _safe_float(text: str) -> Optional[float]:
    try:
        value = float(text.strip())
        if np.isfinite(value):
            return value
        return None
    except Exception:
        return None


def load_iteration_points(csv_path: Path) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """Load sd, bp, dl_ADFbuilding points from iteration rows only."""
    with csv_path.open("r", newline="") as f:
        rows = list(csv.reader(f))

    header_idx: Optional[int] = None
    sd_col: Optional[int] = None
    bp_col: Optional[int] = None
    dl_col: Optional[int] = None

    for i, row in enumerate(rows):
        normalized = [c.strip() for c in row]
        if "sd" in normalized and "bp" in normalized and "dl_ADFbuilding" in normalized:
            header_idx = i
            sd_col = normalized.index("sd")
            bp_col = normalized.index("bp")
            dl_col = normalized.index("dl_ADFbuilding")
            break

    if header_idx is None or sd_col is None or bp_col is None or dl_col is None:
        raise ValueError(f"Could not find objective header row in {csv_path}")

    iter_vals: List[int] = []
    sd_vals: List[float] = []
    bp_vals: List[float] = []
    dl_vals: List[float] = []

    started_iterations = False
    for row in rows[header_idx + 1 :]:
        if not row:
            if started_iterations:
                break
            continue

        first = row[0].strip() if len(row) > 0 else ""
        if not first.isdigit():
            if started_iterations:
                break
            continue

        started_iterations = True

        if len(row) <= max(sd_col, bp_col, dl_col):
            continue

        sd = _safe_float(row[sd_col])
        bp = _safe_float(row[bp_col])
        dl = _safe_float(row[dl_col])

        if sd is None or bp is None or dl is None:
            continue

        iter_vals.append(int(first))
        sd_vals.append(sd)
        bp_vals.append(bp)
        dl_vals.append(dl)

    if not sd_vals:
        raise ValueError(f"No iteration objective points found in {csv_path}")

    return np.array(iter_vals), np.array(sd_vals), np.array(bp_vals), np.array(dl_vals)


def load_run_data(run_dir: Path, csv_name: str) -> RunData:
    csv_path = run_dir / csv_name
    iterations, sd, bp, dl_adf_inv = load_iteration_points(csv_path)
    return RunData(
        design_key=design_key_from_run_dir_name(run_dir.name),
        run_dir=run_dir,
        csv_path=csv_path,
        iterations=iterations,
        sd=sd,
        bp=bp,
        dl_adf_inv=dl_adf_inv,
    )


def sanitize_filename(text: str) -> str:
    safe = []
    for ch in text:
        if ch.isalnum() or ch in "-_.":
            safe.append(ch)
        else:
            safe.append("_")
    return "".join(safe)


def split_design_key(design_key: str) -> Tuple[str, str]:
    """Split key into (base_design, objective_variant).

    Examples:
      design_1_bp -> (design_1, bp)
      design_1_sdbpdl -> (design_1, sdbpdl)
      design_1_sdbpdl_0.333333-... -> (design_1, sdbpdl_weighted)
    """
    m = re.match(r"^(design_\d+)_(.+)$", design_key)
    if not m:
        return design_key, "unknown"

    base = m.group(1)
    tail = m.group(2)

    if tail == "sd":
        return base, "sd"
    if tail == "bp":
        return base, "bp"
    if tail == "dl":
        return base, "dl"
    if tail == "sdbpdl":
        return base, "sdbpdl"
    if tail.startswith("sdbpdl_"):
        return base, "sdbpdl_weighted"
    return base, "unknown"


def pareto_front_mask(sd: np.ndarray, bp: np.ndarray, dl: np.ndarray) -> np.ndarray:
    """Return mask for non-dominated points (min sd, min bp, max dl)."""
    values = np.column_stack([sd, bp, -dl])
    n = values.shape[0]
    mask = np.ones(n, dtype=bool)
    for i in range(n):
        if not mask[i]:
            continue
        is_dominated = np.any(
            np.all(values <= values[i], axis=1) & np.any(values < values[i], axis=1)
        )
        if is_dominated:
            mask[i] = False
    return mask


def _iteration_bounds_for_runs(runs: Iterable[RunData]) -> Tuple[float, float]:
    mins: List[float] = []
    maxs: List[float] = []
    for run in runs:
        if run.iterations.size == 0:
            continue
        mins.append(float(np.min(run.iterations)))
        maxs.append(float(np.max(run.iterations)))
    if not mins:
        return 0.0, 1.0
    lo = min(mins)
    hi = max(maxs)
    if np.isclose(lo, hi):
        hi = lo + 1.0
    return lo, hi


def _draw_pareto_line(ax: plt.Axes, run_data: RunData, color: str, label: str) -> None:
    front = pareto_front_mask(run_data.sd, run_data.bp, run_data.dl_adf_inv)
    if not np.any(front):
        return
    idx = np.where(front)[0]
    idx = idx[np.argsort(run_data.iterations[idx])]
    ax.plot(
        run_data.sd[idx],
        run_data.bp[idx],
        run_data.dl_adf_inv[idx],
        color=color,
        linewidth=1.4,
        alpha=0.95,
        label=label,
    )


def _compute_axis_limits(
    values: np.ndarray,
    data_low_frac: float = 0.10,
    data_high_frac: float = 0.90,
) -> Tuple[float, float]:
    """Compute limits so data falls between data_low_frac and data_high_frac of the axis."""
    arr = np.asarray(values, dtype=float)
    arr = arr[np.isfinite(arr)]
    if arr.size == 0:
        return 0.0, 1.0

    vmin = float(np.min(arr))
    vmax = float(np.max(arr))

    if np.isclose(vmin, vmax):
        # Degenerate case: create a symmetric window around the constant value.
        half_span = max(abs(vmin), 1.0) * 0.5
        return vmin - half_span, vmax + half_span

    data_span = vmax - vmin
    axis_span = data_span / (data_high_frac - data_low_frac)
    lower = vmin - data_low_frac * axis_span
    upper = vmax + (1.0 - data_high_frac) * axis_span
    return lower, upper


def _apply_axis_data_window_10_90(
    ax: plt.Axes,
    sd_values: np.ndarray,
    bp_values: np.ndarray,
    dl_values: np.ndarray,
) -> None:
    """Set x/y/z limits so data uses the 10%-90% central region of each axis."""
    x_lo, x_hi = _compute_axis_limits(sd_values)
    y_lo, y_hi = _compute_axis_limits(bp_values)
    z_lo, z_hi = _compute_axis_limits(dl_values)

    # Apply final axis directions directly: x and z reversed by design.
    ax.set_xlim(x_hi, x_lo)
    ax.set_ylim(y_lo, y_hi)
    ax.set_zlim(z_hi, z_lo)

    # Prevent subsequent artists/layout operations from changing limits.
    ax.set_autoscale_on(False)


def _save_or_show_figure(fig: plt.Figure, output_file: Path, dpi: int, interactive: bool) -> None:
    """Show interactively (optional) and save figure to disk."""
    fig.tight_layout()
    if interactive:
        plt.show(block=True)
    fig.savefig(output_file, dpi=dpi)
    plt.close(fig)


def plot_design_comparison(
    design_key: str,
    modify_data: RunData,
    split_delete_data: RunData,
    output_file: Path,
    dpi: int,
    color_mode: str,
    show_pareto_front: bool,
    interactive: bool,
) -> None:
    fig = plt.figure(figsize=(7.5, 6.0))
    ax = fig.add_subplot(111, projection="3d")

    all_sd = np.concatenate([modify_data.sd, split_delete_data.sd])
    all_bp = np.concatenate([modify_data.bp, split_delete_data.bp])
    all_dl = np.concatenate([modify_data.dl_adf_inv, split_delete_data.dl_adf_inv])

    if color_mode == "method":
        ax.scatter(
            modify_data.sd,
            modify_data.bp,
            modify_data.dl_adf_inv,
            color="C0",
            marker="o",
            s=26,
            alpha=0.85,
            label=f"modify (n={len(modify_data.sd)})",
        )
        ax.scatter(
            split_delete_data.sd,
            split_delete_data.bp,
            split_delete_data.dl_adf_inv,
            color="C1",
            marker="^",
            s=28,
            alpha=0.85,
            label=f"split/delete (n={len(split_delete_data.sd)})",
        )
    else:
        vmin, vmax = _iteration_bounds_for_runs([modify_data, split_delete_data])
        norm = Normalize(vmin=vmin, vmax=vmax)
        cmap = plt.get_cmap("YlOrRd")

        ax.scatter(
            modify_data.sd,
            modify_data.bp,
            modify_data.dl_adf_inv,
            c=modify_data.iterations,
            cmap=cmap,
            norm=norm,
            marker="o",
            s=30,
            alpha=0.90,
            edgecolors="black",
            linewidths=0.25,
        )
        ax.scatter(
            split_delete_data.sd,
            split_delete_data.bp,
            split_delete_data.dl_adf_inv,
            c=split_delete_data.iterations,
            cmap=cmap,
            norm=norm,
            marker="^",
            s=32,
            alpha=0.90,
            edgecolors="black",
            linewidths=0.25,
        )
        sm = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
        sm.set_array([])
        cbar = fig.colorbar(sm, ax=ax, fraction=0.03, pad=0.07)
        cbar.set_label("Iteration number")
        ax.legend(
            handles=[
                Line2D([0], [0], marker="o", color="none", markerfacecolor="#888888", markeredgecolor="black", markersize=7, label="modify"),
                Line2D([0], [0], marker="^", color="none", markerfacecolor="#888888", markeredgecolor="black", markersize=7, label="split/delete"),
            ],
            loc="upper left",
            bbox_to_anchor=(0.0, 1.0),
        )

    if show_pareto_front:
        _draw_pareto_line(ax, modify_data, color="C0", label="modify Pareto front")
        _draw_pareto_line(ax, split_delete_data, color="C1", label="split/delete Pareto front")

    ax.set_xlabel(AXIS_LABELS["sd"])
    ax.set_ylabel(AXIS_LABELS["bp"])
    ax.set_zlabel(AXIS_LABELS["dl_ADFbuilding"])
    _apply_axis_data_window_10_90(ax, all_sd, all_bp, all_dl)

    ax.set_title(f"Objective 3D comparison: {design_key}")
    if color_mode == "method" or show_pareto_front:
        ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.0))

    _save_or_show_figure(fig, output_file, dpi, interactive)


def group_keys_by_base_design(keys: Iterable[str]) -> Dict[str, List[str]]:
    grouped: Dict[str, List[str]] = {}
    for key in keys:
        base, _ = split_design_key(key)
        grouped.setdefault(base, []).append(key)
    return grouped


def plot_combined_objectives_for_base_design(
    base_design: str,
    modify_runs: Dict[str, RunData],
    split_runs: Dict[str, RunData],
    output_file: Path,
    dpi: int,
    color_mode: str,
    show_pareto_front: bool,
    interactive: bool,
) -> None:
    fig = plt.figure(figsize=(7.5, 6.0))
    ax = fig.add_subplot(111, projection="3d")

    all_runs = list(modify_runs.values()) + list(split_runs.values())
    vmin, vmax = _iteration_bounds_for_runs(all_runs)
    norm = Normalize(vmin=vmin, vmax=vmax)
    cmap = plt.get_cmap("YlOrRd")

    all_sd: List[np.ndarray] = []
    all_bp: List[np.ndarray] = []
    all_dl: List[np.ndarray] = []

    # Color shows modification set; marker shows objective variant.
    for objective in OBJECTIVE_ORDER:
        marker = OBJECTIVE_MARKERS[objective]
        label_obj = OBJECTIVE_LABELS[objective]

        if objective in modify_runs:
            d = modify_runs[objective]
            all_sd.append(d.sd)
            all_bp.append(d.bp)
            all_dl.append(d.dl_adf_inv)
            if color_mode == "method":
                ax.scatter(
                    d.sd,
                    d.bp,
                    d.dl_adf_inv,
                    color="C0",
                    marker=marker,
                    s=26,
                    alpha=0.85,
                    label=f"modify {label_obj} (n={len(d.sd)})",
                )
            else:
                ax.scatter(
                    d.sd,
                    d.bp,
                    d.dl_adf_inv,
                    c=d.iterations,
                    cmap=cmap,
                    norm=norm,
                    marker=marker,
                    s=28,
                    alpha=0.9,
                    edgecolors="black",
                    linewidths=0.25,
                    label=f"modify {label_obj} (n={len(d.sd)})",
                )
            if show_pareto_front:
                _draw_pareto_line(ax, d, color="C0", label=f"modify {label_obj} Pareto")

        if objective in split_runs:
            d = split_runs[objective]
            all_sd.append(d.sd)
            all_bp.append(d.bp)
            all_dl.append(d.dl_adf_inv)
            if color_mode == "method":
                ax.scatter(
                    d.sd,
                    d.bp,
                    d.dl_adf_inv,
                    color="C1",
                    marker=marker,
                    s=26,
                    alpha=0.85,
                    label=f"split/delete {label_obj} (n={len(d.sd)})",
                )
            else:
                ax.scatter(
                    d.sd,
                    d.bp,
                    d.dl_adf_inv,
                    c=d.iterations,
                    cmap=cmap,
                    norm=norm,
                    marker=marker,
                    s=28,
                    alpha=0.9,
                    edgecolors="black",
                    linewidths=0.25,
                    label=f"split/delete {label_obj} (n={len(d.sd)})",
                )
            if show_pareto_front:
                _draw_pareto_line(ax, d, color="C1", label=f"split/delete {label_obj} Pareto")

    ax.set_xlabel(AXIS_LABELS["sd"])
    ax.set_ylabel(AXIS_LABELS["bp"])
    ax.set_zlabel(AXIS_LABELS["dl_ADFbuilding"])
    if all_sd and all_bp and all_dl:
        _apply_axis_data_window_10_90(
            ax,
            np.concatenate(all_sd),
            np.concatenate(all_bp),
            np.concatenate(all_dl),
        )

    ax.set_title(f"Objective 3D comparison (all objectives): {base_design}")
    ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.0), fontsize=8)

    if color_mode == "iteration":
        sm = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
        sm.set_array([])
        cbar = fig.colorbar(sm, ax=ax, fraction=0.03, pad=0.07)
        cbar.set_label("Iteration number")

    _save_or_show_figure(fig, output_file, dpi, interactive)


def filter_keys_by_base_designs(keys: Iterable[str], base_designs: Iterable[str]) -> List[str]:
    selected = {d.strip() for d in base_designs if d.strip()}
    if not selected:
        return sorted(keys)
    out: List[str] = []
    for key in sorted(keys):
        base, _ = split_design_key(key)
        if base in selected:
            out.append(key)
    return out


def plot_combined_selected_designs(
    selected_keys: List[str],
    modify_map: Dict[str, Path],
    split_delete_map: Dict[str, Path],
    csv_name: str,
    output_file: Path,
    dpi: int,
    color_mode: str,
    show_pareto_front: bool,
    interactive: bool,
) -> None:
    fig = plt.figure(figsize=(8.4, 6.8))
    ax = fig.add_subplot(111, projection="3d")

    loaded: List[Tuple[str, str, RunData]] = []
    for key in selected_keys:
        loaded.append((key, "modify", load_run_data(modify_map[key], csv_name)))
        loaded.append((key, "split/delete", load_run_data(split_delete_map[key], csv_name)))

    if not loaded:
        raise ValueError("No selected runs available for combined selected-design plot.")

    method_color = {"modify": "C0", "split/delete": "C1"}
    marker_cycle = ["o", "^", "s", "D", "P", "X", "v", "<", ">"]
    base_design_to_marker: Dict[str, str] = {}
    base_designs = sorted({split_design_key(k)[0] for k, _, _ in loaded})
    for i, base in enumerate(base_designs):
        base_design_to_marker[base] = marker_cycle[i % len(marker_cycle)]

    runs_only = [run for _, _, run in loaded]
    vmin, vmax = _iteration_bounds_for_runs(runs_only)
    norm = Normalize(vmin=vmin, vmax=vmax)
    cmap = plt.get_cmap("YlOrRd")

    for key, method, run in loaded:
        base, _ = split_design_key(key)
        marker = base_design_to_marker.get(base, "o")

        if color_mode == "method":
            ax.scatter(
                run.sd,
                run.bp,
                run.dl_adf_inv,
                color=method_color[method],
                marker=marker,
                s=24,
                alpha=0.80,
            )
        else:
            ax.scatter(
                run.sd,
                run.bp,
                run.dl_adf_inv,
                c=run.iterations,
                cmap=cmap,
                norm=norm,
                marker=marker,
                s=24,
                alpha=0.90,
                edgecolors="black",
                linewidths=0.2,
            )

        if show_pareto_front:
            _draw_pareto_line(ax, run, color=method_color[method], label=f"{method} {key} Pareto")

    _apply_axis_data_window_10_90(
        ax,
        np.concatenate([run.sd for _, _, run in loaded]),
        np.concatenate([run.bp for _, _, run in loaded]),
        np.concatenate([run.dl_adf_inv for _, _, run in loaded]),
    )

    ax.set_xlabel(AXIS_LABELS["sd"])
    ax.set_ylabel(AXIS_LABELS["bp"])
    ax.set_zlabel(AXIS_LABELS["dl_ADFbuilding"])
    ax.set_title("Objective 3D comparison (combined selected designs)")

    method_handles = [
        Line2D([0], [0], marker="o", color="none", markerfacecolor="C0", markersize=7, label="modify"),
        Line2D([0], [0], marker="o", color="none", markerfacecolor="C1", markersize=7, label="split/delete"),
    ]
    design_handles = [
        Line2D([0], [0], marker=base_design_to_marker[b], color="black", linestyle="None", markerfacecolor="none", markersize=7, label=b)
        for b in base_designs
    ]
    ax.legend(handles=method_handles + design_handles, loc="upper left", bbox_to_anchor=(0.0, 1.0), fontsize=8)

    if color_mode == "iteration":
        sm = plt.cm.ScalarMappable(norm=norm, cmap=cmap)
        sm.set_array([])
        cbar = fig.colorbar(sm, ax=ax, fraction=0.03, pad=0.07)
        cbar.set_label("Iteration number")

    _save_or_show_figure(fig, output_file, dpi, interactive)


def filter_keys(keys: Iterable[str], design_filter: str) -> List[str]:
    key_list = sorted(keys)
    if not design_filter:
        return key_list
    needle = design_filter.strip().lower()
    return [k for k in key_list if needle in k.lower()]


def main() -> int:
    args = parse_args()

    modify_map = collect_latest_run_dirs(args.modify_root, args.csv_name)
    split_delete_map = collect_latest_run_dirs(args.split_delete_root, args.csv_name)

    common_keys = sorted(set(modify_map.keys()) & set(split_delete_map.keys()))
    common_keys = filter_keys(common_keys, args.design_filter)
    common_keys = filter_keys_by_base_designs(common_keys, args.base_designs)

    if not common_keys:
        print("No matching design keys found between the two roots.")
        return 1

    args.output_dir.mkdir(parents=True, exist_ok=True)

    generated = 0
    generated_combined = 0
    failed: List[Tuple[str, str]] = []

    for key in common_keys:
        try:
            modify_data = load_run_data(modify_map[key], args.csv_name)
            split_delete_data = load_run_data(split_delete_map[key], args.csv_name)

            out_name = f"objective3DPlot_compare_{sanitize_filename(key)}.png"
            out_path = args.output_dir / out_name

            plot_design_comparison(
                design_key=key,
                modify_data=modify_data,
                split_delete_data=split_delete_data,
                output_file=out_path,
                dpi=args.dpi,
                color_mode=args.color_mode,
                show_pareto_front=args.show_pareto_front,
                interactive=args.interactive,
            )
            generated += 1
            print(f"[OK] {key} -> {out_path}")
        except Exception as exc:
            failed.append((key, str(exc)))
            print(f"[FAIL] {key}: {exc}")

    if not args.skip_combined_objectives:
        keys_by_base = group_keys_by_base_design(common_keys)
        for base_design in sorted(keys_by_base.keys()):
            key_group = keys_by_base[base_design]
            modify_runs: Dict[str, RunData] = {}
            split_runs: Dict[str, RunData] = {}

            try:
                for key in key_group:
                    _, objective = split_design_key(key)
                    if objective == "unknown":
                        continue
                    if objective not in modify_runs:
                        modify_runs[objective] = load_run_data(modify_map[key], args.csv_name)
                    if objective not in split_runs:
                        split_runs[objective] = load_run_data(split_delete_map[key], args.csv_name)

                if not modify_runs and not split_runs:
                    raise ValueError("No recognized objective variants available.")

                out_name = f"objective3DPlot_compare_combined_{sanitize_filename(base_design)}.png"
                out_path = args.output_dir / out_name

                plot_combined_objectives_for_base_design(
                    base_design=base_design,
                    modify_runs=modify_runs,
                    split_runs=split_runs,
                    output_file=out_path,
                    dpi=args.dpi,
                    color_mode=args.color_mode,
                    show_pareto_front=args.show_pareto_front,
                    interactive=args.interactive,
                )
                generated_combined += 1
                print(f"[OK] {base_design} (combined) -> {out_path}")
            except Exception as exc:
                failed.append((f"{base_design} (combined)", str(exc)))
                print(f"[FAIL] {base_design} (combined): {exc}")

    if args.combine_selected_designs:
        try:
            suffix_parts: List[str] = []
            if args.base_designs:
                suffix_parts.append("_".join(sorted({b.strip() for b in args.base_designs if b.strip()})))
            elif args.design_filter:
                suffix_parts.append(args.design_filter)
            else:
                suffix_parts.append("all_selected")
            suffix = sanitize_filename("_".join(suffix_parts))
            out_name = f"objective3DPlot_compare_selected_combined_{suffix}.png"
            out_path = args.output_dir / out_name

            plot_combined_selected_designs(
                selected_keys=common_keys,
                modify_map=modify_map,
                split_delete_map=split_delete_map,
                csv_name=args.csv_name,
                output_file=out_path,
                dpi=args.dpi,
                color_mode=args.color_mode,
                show_pareto_front=args.show_pareto_front,
                interactive=args.interactive,
            )
            generated_combined += 1
            print(f"[OK] selected designs (combined) -> {out_path}")
        except Exception as exc:
            failed.append(("selected designs (combined)", str(exc)))
            print(f"[FAIL] selected designs (combined): {exc}")

    print()
    print(f"Generated per-objective plots: {generated}")
    print(f"Generated combined plots:      {generated_combined}")
    print(f"Generated plots total:        {generated + generated_combined}")
    print(f"Failed plots:    {len(failed)}")

    missing_in_split = sorted(set(modify_map.keys()) - set(split_delete_map.keys()))
    missing_in_modify = sorted(set(split_delete_map.keys()) - set(modify_map.keys()))

    print(f"Only in modify root:      {len(missing_in_split)}")
    print(f"Only in split/delete root:{len(missing_in_modify)}")

    if failed:
        print("\nFailures:")
        for key, err in failed:
            print(f"  - {key}: {err}")

    return 0 if (generated + generated_combined) > 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
