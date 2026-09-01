#!/usr/bin/env python3
"""Analyze metric coupling across iteration-to-iteration transitions.

For each run CSV found under one or more input roots, this script extracts the iteration table
with the metrics columns and builds transition records (i -> i+1). It then answers:
"When metric X improves from i to i+1, what tends to happen to metric Y?"

Outputs:
- transitions_wide.csv: one row per transition with deltas 
  (change in objective performance variable both absolute and relative)
  and improvement flags (bool: is objective improved or not?).
- transitions_by_focus.csv: one row per transition per focus metric.
  So each transition is repeated once per focus metric.
  This way, we can easily condition on focus metric improvement and analyze affected metrics.
- coupling_summary.csv: aggregated outcomes. Per focus objective (i.e. metric), 
  we summarize the affected objectives.
  outputs include:
  - n_transitions: how many transitions were observed for this focus-affected pair?
  - n_focus_improved: how many of those transitions had the focus metric improve?
  - p_focus_improved: how often does the focus metric improve across all transitions?
  - mean_abs_delta_if_focus_improved: when the focus metric improves, how much does the affected metric change on average (absolute delta)?
  - median_abs_delta_if_focus_improved: same but median.
  - mean_rel_delta_if_focus_improved: same but relative delta.
  - median_rel_delta_if_focus_improved: same but median.
  - p_affected_improved_if_focus_improved: when the focus metric improves, how often does the affected metric also improve?
  - mean_abs_delta_if_focus_not_improved: when the focus metric does NOT improve, how much does the affected metric change on average (absolute delta)?
  - median_abs_delta_if_focus_not_improved: same but median.
  - mean_rel_delta_if_focus_not_improved: same but relative delta.
  - median_rel_delta_if_focus_not_improved: same but median.
  - p_affected_improved_if_focus_not_improved: when the focus metric does NOT improve, how often does the affected metric improve?

- heatmap_co_improvement.png: P(affected improves | focus improves).
- heatmap_median_relative_effect.png: median relative change of affected metric
  when focus metric improves.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
import textwrap
from typing import Dict, Iterable, List, Optional

import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.ticker import FormatStrFormatter
import numpy as np
import pandas as pd


DEFAULT_METRICS = ["sd", "bp", "dl_ADFbuilding", "dl_score"]
EPS = 1e-12

METRIC_DISPLAY_NAMES = {
    "sd": "structural (strain energy [Nmm])",
    "bp": "thermal (heating and cooling energy [kWh])",
    "dl_ADFbuilding": "daylight (average daylight factor [%])",
    "dl_score": "daylight (BS EN 17037-2018 section B.3.2 score [-])",
}

METRIC_AXIS_LABELS = {
    "sd": "strain energy [Nmm]",
    "bp": "heating and cooling energy [kWh]",
    "dl_ADFbuilding": "average daylight factor [%]",
    "dl_score": "BS EN 17037-2018 section B.3.2 score [-]",
}

HEATMAP_METRIC_LABELS = {
    "sd": "sd",
    "bp": "bp",
    "dl_ADFbuilding": "dl1",
    "dl_score": "dl2",
}

HEATMAP_LEGEND_LABELS = {
    "sd": "sd: strain energy [N/mm]",
    "bp": "bp: heating and cooling energy [kWh]",
    "dl_ADFbuilding": "dl1: average daylight factor [%]",
    "dl_score": "dl2: BS EN 17037-2018 section B.3.2 score [-]",
}

METRIC_COLORS = {
    "sd": "#2E7D32",
    "bp": "#1565C0",
    "dl_ADFbuilding": "#C62828",
    "dl_score": "#6A1B9A",
}


def metric_legend_handles(
    metrics: List[str],
    label_wrap_width: Optional[int] = None,
) -> List[Patch]:
    return [
        Patch(
            facecolor=METRIC_COLORS.get(metric, "#666666"),
            alpha=0.42,
            label=(
                textwrap.fill(METRIC_AXIS_LABELS.get(metric, metric), label_wrap_width)
                if label_wrap_width is not None
                else METRIC_AXIS_LABELS.get(metric, metric)
            ),
        )
        for metric in metrics
    ]


def focus_axis_label(metric: str) -> str:
    """Format a focus label as two similarly sized lines."""
    words = f"focus: {METRIC_DISPLAY_NAMES.get(metric, metric)} relative delta".split()
    split_index = min(
        range(1, len(words)),
        key=lambda index: abs(
            len(" ".join(words[:index])) - len(" ".join(words[index:]))
        ),
    )
    return " ".join(words[:split_index]) + "\n" + " ".join(words[split_index:])


@dataclass
class ParsedRun:
    run_id: str
    file_path: Path
    data: pd.DataFrame


def _safe_float(value: str) -> Optional[float]:
    try:
        return float(value.strip())
    except Exception:
        return None


def _find_metric_header(rows: List[List[str]], metrics: List[str]) -> Optional[int]:
    target = set(metrics)
    for i, row in enumerate(rows):
        cleaned = [c.strip() for c in row]
        if target.issubset(set(cleaned)):
            return i
    return None


def parse_single_csv(file_path: Path, metrics: List[str]) -> Optional[pd.DataFrame]:
    with file_path.open("r", newline="") as f:
        rows = list(csv.reader(f))

    header_idx = _find_metric_header(rows, metrics)
    if header_idx is None:
        return None

    header = [c.strip() for c in rows[header_idx]]
    metric_col_idx: Dict[str, int] = {}
    for m in metrics:
        if m not in header:
            return None
        metric_col_idx[m] = header.index(m)

    records = []
    for row in rows[header_idx + 1 :]:
        if not row:
            break
        iter_cell = row[0].strip() if len(row) > 0 else ""
        if not iter_cell.isdigit():
            break

        metric_values: Dict[str, float] = {}
        valid = True
        for m in metrics:
            idx = metric_col_idx[m]
            if idx >= len(row):
                valid = False
                break
            val = _safe_float(row[idx])
            if val is None:
                valid = False
                break
            metric_values[m] = val

        if not valid:
            continue

        records.append({"iteration": int(iter_cell), **metric_values})

    if len(records) < 2:
        return None

    out = pd.DataFrame.from_records(records)
    out = out.sort_values("iteration").drop_duplicates(subset=["iteration"], keep="last")
    return out.reset_index(drop=True)


def load_runs(root_dirs: Iterable[Path], metrics: List[str]) -> List[ParsedRun]:
    parsed_runs: List[ParsedRun] = []
    for root_dir in root_dirs:
        for file_path in sorted(root_dir.rglob("*.csv")):
            if file_path.stem == "combined_data":
                continue
            df = parse_single_csv(file_path, metrics)
            if df is None:
                continue
            run_id = file_path.parent.name
            parsed_runs.append(ParsedRun(run_id=run_id, file_path=file_path, data=df))
    return parsed_runs


def build_transition_tables(
    runs: Iterable[ParsedRun],
    metrics: List[str],
    maximize_metrics: List[str],
) -> tuple[pd.DataFrame, pd.DataFrame]:
    maximize_set = set(maximize_metrics)

    wide_rows = []
    focus_rows = []

    for run in runs:
        df = run.data
        for i in range(len(df) - 1):
            cur = df.iloc[i]
            nxt = df.iloc[i + 1]

            base = {
                "run_id": run.run_id,
                "file_path": str(run.file_path),
                "from_iteration": int(cur["iteration"]),
                "to_iteration": int(nxt["iteration"]),
            }

            for m in metrics:
                abs_delta = float(nxt[m] - cur[m])
                rel_delta = abs_delta / max(abs(float(cur[m])), EPS)
                improved = abs_delta > 0 if m in maximize_set else abs_delta < 0
                base[f"delta_{m}"] = abs_delta
                base[f"rel_delta_{m}"] = rel_delta
                base[f"improved_{m}"] = improved

            wide_rows.append(base)

            for focus in metrics:
                focus_abs_delta = float(nxt[focus] - cur[focus])
                focus_rel_delta = focus_abs_delta / max(abs(float(cur[focus])), EPS)
                focus_improved = (
                    focus_abs_delta > 0 if focus in maximize_set else focus_abs_delta < 0
                )

                row = {
                    "run_id": run.run_id,
                    "file_path": str(run.file_path),
                    "from_iteration": int(cur["iteration"]),
                    "to_iteration": int(nxt["iteration"]),
                    "focus_metric": focus,
                    "focus_abs_delta": focus_abs_delta,
                    "focus_rel_delta": focus_rel_delta,
                    "focus_improved": focus_improved,
                }

                for affected in metrics:
                    affected_abs_delta = float(nxt[affected] - cur[affected])
                    affected_rel_delta = affected_abs_delta / max(
                        abs(float(cur[affected])), EPS
                    )
                    affected_improved = (
                        affected_abs_delta > 0
                        if affected in maximize_set
                        else affected_abs_delta < 0
                    )
                    row[f"affected_abs_delta_{affected}"] = affected_abs_delta
                    row[f"affected_rel_delta_{affected}"] = affected_rel_delta
                    row[f"affected_improved_{affected}"] = affected_improved

                focus_rows.append(row)

    return pd.DataFrame(wide_rows), pd.DataFrame(focus_rows)


def summarize_coupling(
    focus_df: pd.DataFrame,
    metrics: List[str],
) -> pd.DataFrame:
    rows = []

    for focus in metrics:
        subset_focus = focus_df[focus_df["focus_metric"] == focus]
        if subset_focus.empty:
            continue

        improved_subset = subset_focus[subset_focus["focus_improved"]]
        not_improved_subset = subset_focus[~subset_focus["focus_improved"]]

        for affected in metrics:
            col_abs = f"affected_abs_delta_{affected}"
            col_rel = f"affected_rel_delta_{affected}"
            col_imp = f"affected_improved_{affected}"

            row = {
                "focus_metric": focus,
                "affected_metric": affected,
                "n_transitions": int(len(subset_focus)),
                "n_focus_improved": int(len(improved_subset)),
                "p_focus_improved": float(len(improved_subset) / len(subset_focus)),
                "mean_abs_delta_if_focus_improved": float(improved_subset[col_abs].mean())
                if len(improved_subset)
                else np.nan,
                "median_abs_delta_if_focus_improved": float(improved_subset[col_abs].median())
                if len(improved_subset)
                else np.nan,
                "mean_rel_delta_if_focus_improved": float(improved_subset[col_rel].mean())
                if len(improved_subset)
                else np.nan,
                "median_rel_delta_if_focus_improved": float(improved_subset[col_rel].median())
                if len(improved_subset)
                else np.nan,
                "p_affected_improved_if_focus_improved": float(improved_subset[col_imp].mean())
                if len(improved_subset)
                else np.nan,
                "mean_abs_delta_if_focus_not_improved": float(not_improved_subset[col_abs].mean())
                if len(not_improved_subset)
                else np.nan,
                "median_abs_delta_if_focus_not_improved": float(not_improved_subset[col_abs].median())
                if len(not_improved_subset)
                else np.nan,
                "mean_rel_delta_if_focus_not_improved": float(not_improved_subset[col_rel].mean())
                if len(not_improved_subset)
                else np.nan,
                "median_rel_delta_if_focus_not_improved": float(not_improved_subset[col_rel].median())
                if len(not_improved_subset)
                else np.nan,
                "p_affected_improved_if_focus_not_improved": float(
                    not_improved_subset[col_imp].mean()
                )
                if len(not_improved_subset)
                else np.nan,
            }

            row["delta_p_affected_improved"] = (
                row["p_affected_improved_if_focus_improved"]
                - row["p_affected_improved_if_focus_not_improved"]
                if not np.isnan(row["p_affected_improved_if_focus_improved"])
                and not np.isnan(row["p_affected_improved_if_focus_not_improved"])
                else np.nan
            )

            rows.append(row)

    return pd.DataFrame(rows)


def plot_heatmap(
    matrix_df: pd.DataFrame,
    metrics: List[str],
    title: str,
    colorbar_label: str,
    output_path: Path,
    cmap: str = "coolwarm",
    value_fmt: str = ".2f",
    figsize: tuple[float, float] = (8.5, 6.5),
    metric_labels: Optional[Dict[str, str]] = None,
    x_label_rotation: float = 30,
    font_size: Optional[float] = None,
    axes_bounds: Optional[tuple[float, float, float, float]] = None,
    colorbar_bounds: Optional[tuple[float, float, float, float]] = None,
    x_axis_label: str = "Affected metric",
    y_axis_label: str = "Focus metric (conditioned on improvement)",
    legend_labels: Optional[Dict[str, str]] = None,
) -> None:
    mat = np.full((len(metrics), len(metrics)), np.nan)
    for i, focus in enumerate(metrics):
        for j, affected in enumerate(metrics):
            mask = (
                (matrix_df["focus_metric"] == focus)
                & (matrix_df["affected_metric"] == affected)
            )
            if mask.any():
                mat[i, j] = float(matrix_df.loc[mask, "value"].iloc[0])

    fig, ax = plt.subplots(figsize=figsize)
    im = ax.imshow(mat, cmap=cmap, aspect="auto")

    display_labels = [metric_labels.get(metric, metric) for metric in metrics] if metric_labels else metrics
    ax.set_xticks(np.arange(len(metrics)))
    ax.set_yticks(np.arange(len(metrics)))
    ax.set_xticklabels(display_labels, rotation=x_label_rotation, ha="right" if x_label_rotation else "center")
    ax.set_yticklabels([textwrap.fill(label, width=22) for label in display_labels])
    ax.set_xlabel(x_axis_label)
    ax.set_ylabel(y_axis_label)
    if title:
        ax.set_title(title)
    if font_size is not None:
        ax.tick_params(axis="both", labelsize=font_size)
        ax.xaxis.label.set_size(font_size)
        ax.yaxis.label.set_size(font_size)

    for i in range(len(metrics)):
        for j in range(len(metrics)):
            v = mat[i, j]
            if np.isnan(v):
                text = "NA"
            else:
                text = format(v, value_fmt)
            ax.text(j, i, text, ha="center", va="center", fontsize=font_size or 9)

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label(colorbar_label)
    if font_size is not None:
        cbar.ax.tick_params(labelsize=font_size)
        cbar.ax.yaxis.label.set_size(font_size)

    if axes_bounds is None:
        fig.tight_layout()
    else:
        ax.set_position(axes_bounds)
        if colorbar_bounds is not None:
            cbar.ax.set_position(colorbar_bounds)
    if legend_labels is not None:
        legend_handles = [
            Patch(
                facecolor="none",
                edgecolor="none",
                label=textwrap.fill(legend_labels.get(metric, metric), width=27),
            )
            for metric in metrics
        ]
        fig.legend(
            handles=legend_handles,
            loc="lower center",
            bbox_to_anchor=(0.5, -0.02),
            ncol=2,
            frameon=False,
            handlelength=0,
            handletextpad=0,
            prop={"size": font_size or 8},
        )
    fig.savefig(output_path, dpi=220)
    plt.close(fig)


def plot_per_pair_boxplot_panel(
    focus_df: pd.DataFrame,
    metrics: List[str],
    output_path: Path,
) -> None:
    """Create one subplot per focus metric with all affected metrics.

    Each subplot compares distributions of affected relative deltas for each metric
    (including the focus metric itself) under two conditions:
    - focus improved
    - focus not improved
    """
    # Panel grid layout: width is 170 mm; change ncols or the height factor to alter panel arrangement and size.
    n = len(metrics)
    ncols = min(2, n)
    nrows = int(np.ceil(n / ncols))
    fig, axes = plt.subplots(
        nrows,
        ncols,
        figsize=(170 / 25.4, 2.5 * nrows),
        sharey=False,
        squeeze=False,
    )
    axes_flat = axes.ravel()

    for i, focus in enumerate(metrics):
        ax = axes_flat[i]
        sub = focus_df[focus_df["focus_metric"] == focus]
        affected_metrics = list(metrics)

        improved_groups = []
        not_improved_groups = []
        # Horizontal spacing within and between the two condition sections.
        box_width = 0.7
        group_gap = 1.2

        for affected in affected_metrics:
            col = f"affected_rel_delta_{affected}"
            improved_vals = sub.loc[sub["focus_improved"], col].to_numpy(dtype=float)
            not_improved_vals = sub.loc[~sub["focus_improved"], col].to_numpy(dtype=float)

            improved_vals = improved_vals[np.isfinite(improved_vals)]
            not_improved_vals = not_improved_vals[np.isfinite(not_improved_vals)]

            # Winsorize for visual robustness while preserving sign/scale interpretation.
            combined = (
                np.concatenate([improved_vals, not_improved_vals])
                if (len(improved_vals) + len(not_improved_vals)) > 0
                else np.array([])
            )
            if len(combined) > 0:
                lo = np.percentile(combined, 2.5)
                hi = np.percentile(combined, 97.5)
                improved_vals = np.clip(improved_vals, lo, hi)
                not_improved_vals = np.clip(not_improved_vals, lo, hi)

            improved_groups.append(improved_vals)
            not_improved_groups.append(not_improved_vals)

        groups = improved_groups + not_improved_groups
        positions = list(range(len(affected_metrics)))
        positions.extend(
            metric_idx + len(affected_metrics) + group_gap
            for metric_idx in range(len(affected_metrics))
        )
        colors = [
            METRIC_COLORS.get(metric, "#666666")
            for metric in affected_metrics + affected_metrics
        ]

        bp = ax.boxplot(
            groups,
            positions=positions,
            patch_artist=True,
            widths=box_width,
            showfliers=False,
            boxprops=dict(linewidth=0.6),
            whiskerprops=dict(linewidth=0.6),
            capprops=dict(linewidth=0.6),
            medianprops=dict(linewidth=0.6),
            flierprops=dict(linewidth=0.6),
        )

        for patch, color in zip(bp["boxes"], colors):
            patch.set_facecolor(color)
            patch.set_alpha(0.45)

        for median in bp["medians"]:
            median.set_color("#222222")
            median.set_linewidth(0.6)

        section_boundary = len(affected_metrics) - 0.5 + group_gap / 2
        ax.axvspan(-0.5, section_boundary, facecolor="#2F7D32", alpha=0.08, zorder=0)
        ax.axvspan(section_boundary, positions[-1] + 0.5, facecolor="#C62828", alpha=0.08, zorder=0)
        ax.axvline(section_boundary, color="#888888", linewidth=0.6, linestyle=":", zorder=0)
        ax.text(
            section_boundary - 0.08,
            0.98,
            "Focus\nimproved",
            transform=ax.get_xaxis_transform(),
            ha="right",
            va="top",
            fontsize=8,
            color="#1B5E20",
        )
        ax.text(
            positions[-1] + 0.42,
            0.98,
            "Focus not\nimproved",
            transform=ax.get_xaxis_transform(),
            ha="right",
            va="top",
            fontsize=8,
            color="#8E1B1B",
        )
        ax.axhline(0.0, color="#555555", linewidth=0.6, linestyle="--")
        ax.set_ylabel(focus_axis_label(focus))
        ax.yaxis.label.set_size(8)
        ax.yaxis.set_major_formatter(FormatStrFormatter("%.0f"))
        ax.set_xticks(positions)
        ax.set_xticklabels([""] * len(positions))
        ax.tick_params(axis="both", labelsize=8)
        ax.tick_params(axis="both", width=0.6)
        ax.tick_params(axis="x", length=0)
        for spine in ax.spines.values():
            spine.set_linewidth(0.6)
        ax.grid(axis="y", alpha=0.25)

    for ax in axes_flat[n:]:
        ax.set_visible(False)

    # Bottom legend layout: one row of wrapped labels fits the fixed 170 mm figure width.
    fig.legend(
        handles=metric_legend_handles(metrics, label_wrap_width=22.5),
        loc="lower center",
        bbox_to_anchor=(0.5, 0.00),
        ncol=4,
        frameon=False,
        prop={"size": 8},
    )
    fig.tight_layout(rect=[0, 0.075, 1, 0.988]) # reserves the height at the bottom for legend
    fig.savefig(output_path, dpi=260)
    plt.close(fig)


def plot_condition_only_boxplot_panel(
    focus_df: pd.DataFrame,
    metrics: List[str],
    focus_improved_value: bool,
    output_path: Path,
) -> None:
    """Create one subplot per focus metric for a single focus condition.

    If focus_improved_value is True, only transitions where focus improved are shown.
    If focus_improved_value is False, only transitions where focus did not improve are shown.
    Each subplot contains boxplots for all metrics (focus + affected).
    """
    # Panel grid layout: change ncols or the figsize factors to alter panel arrangement and size.
    n = len(metrics)
    ncols = min(2, n)
    nrows = int(np.ceil(n / ncols))
    fig, axes = plt.subplots(
        nrows,
        ncols,
        figsize=(max(12, 7.0 * ncols), 4.6 * nrows),
        sharey=False,
        squeeze=False,
    )
    axes_flat = axes.ravel()

    for i, focus in enumerate(metrics):
        ax = axes_flat[i]
        sub = focus_df[
            (focus_df["focus_metric"] == focus)
            & (focus_df["focus_improved"] == focus_improved_value)
        ]

        groups = []

        for metric in metrics:
            col = f"affected_rel_delta_{metric}"
            vals = sub[col].to_numpy(dtype=float)
            vals = vals[np.isfinite(vals)]
            if len(vals) > 0:
                lo = np.percentile(vals, 2.5)
                hi = np.percentile(vals, 97.5)
                vals = np.clip(vals, lo, hi)

            groups.append(vals)

        bp = ax.boxplot(
            groups,
            patch_artist=True,
            widths=0.6,
            showfliers=False,
        )

        for patch, metric in zip(bp["boxes"], metrics):
            patch.set_facecolor(METRIC_COLORS.get(metric, "#666666"))
            patch.set_alpha(0.45)

        for median in bp["medians"]:
            median.set_color("#222222")
            median.set_linewidth(1.8)

        ax.axhline(0.0, color="#555555", linewidth=1.0, linestyle="--")
        ax.set_ylabel(focus_axis_label(focus))
        ax.yaxis.set_major_formatter(FormatStrFormatter("%.0f"))
        ax.set_xticks(range(1, len(metrics) + 1))
        ax.set_xticklabels([""] * len(metrics))
        ax.tick_params(axis="x", length=0)
        ax.grid(axis="y", alpha=0.25)

    for ax in axes_flat[n:]:
        ax.set_visible(False)

    fig.suptitle(
        (
            "Relative Effect Distributions per Focus Metric\n"
            "Condition: Focus Metric Improved"
            if focus_improved_value
            else "Relative Effect Distributions per Focus Metric\nCondition: Focus Metric Not Improved"
        ),
        fontsize=15,
        y=0.995,
    )
    # Bottom legend layout: bbox_to_anchor controls its vertical distance from the panels.
    fig.legend(
        handles=metric_legend_handles(metrics),
        loc="lower center",
        bbox_to_anchor=(0.5, 0.01),
        ncol=len(metrics),
        frameon=False,
    )
    fig.tight_layout(rect=[0, 0.07, 1, 0.988])
    fig.savefig(output_path, dpi=260)
    plt.close(fig)


def plot_single_focus_nonfocus_boxplot(
    focus_df: pd.DataFrame,
    metrics: List[str],
    focus_metric: str,
    output_path: Path,
) -> None:
    """Plot one publication-ready graph for a selected focus metric.

    This plot shows all non-focus metrics in one figure, comparing distributions of
    relative deltas for transitions where the focus metric improved vs did not improve.
    """
    if focus_metric not in metrics:
        raise ValueError(f"focus_metric '{focus_metric}' must be in metrics {metrics}")

    affected_metrics = [m for m in metrics if m != focus_metric]
    sub = focus_df[focus_df["focus_metric"] == focus_metric].copy()

    groups = []
    positions = []
    pos = 0.0
    # Horizontal spacing between paired condition boxes and between affected metrics.
    box_width = 0.7
    condition_spacing = 1.0
    group_gap = 1.2

    for metric in affected_metrics:
        col = f"affected_rel_delta_{metric}"
        vals_improved = sub.loc[sub["focus_improved"], col].to_numpy(dtype=float)
        vals_not = sub.loc[~sub["focus_improved"], col].to_numpy(dtype=float)

        vals_improved = vals_improved[np.isfinite(vals_improved)]
        vals_not = vals_not[np.isfinite(vals_not)]

        combined = np.concatenate([vals_improved, vals_not]) if (len(vals_improved) + len(vals_not)) > 0 else np.array([])
        if len(combined) > 0:
            lo = np.percentile(combined, 2.5)
            hi = np.percentile(combined, 97.5)
            vals_improved = np.clip(vals_improved, lo, hi)
            vals_not = np.clip(vals_not, lo, hi)

        groups.extend([vals_improved, vals_not])
        positions.extend([pos, pos + condition_spacing])
        pos += condition_spacing + group_gap

    fig, ax = plt.subplots(figsize=(max(10, 2.9 * len(affected_metrics)), 6.5))
    bp = ax.boxplot(
        groups,
        positions=positions,
        patch_artist=True,
        widths=box_width,
        showfliers=False,
    )

    for patch, metric in zip(bp["boxes"], [metric for metric in affected_metrics for _ in range(2)]):
        patch.set_facecolor(METRIC_COLORS.get(metric, "#666666"))
        patch.set_alpha(0.45)

    for median in bp["medians"]:
        median.set_color("#222222")
        median.set_linewidth(1.8)

    ax.axhline(0.0, color="#555555", linewidth=1.0, linestyle="--")
    ax.set_ylabel("Relative delta (winsorized 2.5%-97.5%)")
    ax.yaxis.set_major_formatter(FormatStrFormatter("%.0f"))
    ax.set_title(
        f"Effect on Non-focus Metrics When {focus_metric} Improves",
        fontsize=14,
    )
    ax.set_xticks(positions)
    ax.set_xticklabels([""] * len(positions))
    ax.tick_params(axis="x", length=0)
    ax.grid(axis="y", alpha=0.25)

    # Bottom legend layout: bbox_to_anchor controls its vertical distance from the plot.
    fig.legend(
        handles=metric_legend_handles(affected_metrics),
        loc="lower center",
        bbox_to_anchor=(0.5, 0.01),
        ncol=len(affected_metrics),
        frameon=False,
    )
    fig.tight_layout(rect=[0, 0.07, 1, 1])
    fig.savefig(output_path, dpi=260)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Analyze how improvements in one metric relate to changes in others."
    )
    parser.add_argument(
        "input_roots",
        nargs="+",
        type=Path,
        help="One or more root directories containing run CSV files (searched recursively).",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("coupling_analysis_output"),
        help="Output directory for generated CSV and plots.",
    )
    parser.add_argument(
        "--metrics",
        nargs="+",
        default=DEFAULT_METRICS,
        help="Metric column names to analyze.",
    )
    parser.add_argument(
        "--maximize-metrics",
        nargs="*",
        default=[],
        help="Metrics for which larger values are considered improvements.",
    )

    args = parser.parse_args()

    input_roots: List[Path] = args.input_roots
    output_dir: Path = args.output_dir
    metrics: List[str] = args.metrics
    maximize_metrics: List[str] = args.maximize_metrics

    output_dir.mkdir(parents=True, exist_ok=True)

    runs = load_runs(input_roots, metrics)
    if not runs:
        raise SystemExit(
            "No parseable CSV runs found. Check input path and metric names in the CSV headers."
        )

    transitions_wide, transitions_by_focus = build_transition_tables(
        runs=runs,
        metrics=metrics,
        maximize_metrics=maximize_metrics,
    )

    summary = summarize_coupling(transitions_by_focus, metrics)

    transitions_wide_path = output_dir / "transitions_wide.csv"
    transitions_by_focus_path = output_dir / "transitions_by_focus.csv"
    summary_path = output_dir / "coupling_summary.csv"

    transitions_wide.to_csv(transitions_wide_path, index=False)
    transitions_by_focus.to_csv(transitions_by_focus_path, index=False)
    summary.to_csv(summary_path, index=False)

    co_improvement_df = summary[["focus_metric", "affected_metric", "p_affected_improved_if_focus_improved"]].rename(
        columns={"p_affected_improved_if_focus_improved": "value"}
    )
    co_improvement_df["value"] *= 100.0
    plot_heatmap(
        matrix_df=co_improvement_df,
        metrics=metrics,
        title="",
        colorbar_label=r"$p_{\mathrm{focus\ improved}}$ [%]",
        output_path=output_dir / "heatmap_co_improvement.png",
        cmap="viridis",
        value_fmt=".0f",
        figsize=(85 / 25.4, 85 / 40),
        metric_labels=HEATMAP_METRIC_LABELS,
        x_label_rotation=90,
        font_size=8,
        axes_bounds=(0.15, 0.49, 0.65, 0.50),
        colorbar_bounds=(0.82, 0.57, 0.04, 0.31),
        x_axis_label="Affected metric",
        y_axis_label="Focus metric",
        legend_labels=HEATMAP_LEGEND_LABELS,
    )

    relative_effect_df = summary[["focus_metric", "affected_metric", "median_rel_delta_if_focus_improved"]].rename(
        columns={"median_rel_delta_if_focus_improved": "value"}
    )
    plot_heatmap(
        matrix_df=relative_effect_df,
        metrics=metrics,
        title="Median Relative Delta of Affected (Focus Improved)",
        colorbar_label="Median relative delta",
        output_path=output_dir / "heatmap_median_relative_effect.png",
        cmap="coolwarm",
        value_fmt=".3f",
    )

    boxplot_path = output_dir / "boxplot_panel_relative_effects.png"
    plot_per_pair_boxplot_panel(
        focus_df=transitions_by_focus,
        metrics=metrics,
        output_path=boxplot_path,
    )

    boxplot_focus_improved_only_path = output_dir / "boxplot_panel_focus_improved_only.png"
    plot_condition_only_boxplot_panel(
        focus_df=transitions_by_focus,
        metrics=metrics,
        focus_improved_value=True,
        output_path=boxplot_focus_improved_only_path,
    )

    boxplot_focus_not_improved_only_path = output_dir / "boxplot_panel_focus_not_improved_only.png"
    plot_condition_only_boxplot_panel(
        focus_df=transitions_by_focus,
        metrics=metrics,
        focus_improved_value=False,
        output_path=boxplot_focus_not_improved_only_path,
    )

    print("Parsed runs:", len(runs))
    print("Transitions:", len(transitions_wide))
    print("Outputs:")
    print(" -", transitions_wide_path)
    print(" -", transitions_by_focus_path)
    print(" -", summary_path)
    print(" -", output_dir / "heatmap_co_improvement.png")
    print(" -", output_dir / "heatmap_median_relative_effect.png")
    print(" -", boxplot_path)
    print(" -", boxplot_focus_improved_only_path)
    print(" -", boxplot_focus_not_improved_only_path)


if __name__ == "__main__":
    main()
