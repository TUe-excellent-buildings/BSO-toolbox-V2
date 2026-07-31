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
from typing import Dict, Iterable, List, Optional

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


DEFAULT_METRICS = ["sd", "bp", "dl_ADFbuilding_inverse", "dl_score"]
EPS = 1e-12


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

    fig, ax = plt.subplots(figsize=(8.5, 6.5))
    im = ax.imshow(mat, cmap=cmap, aspect="auto")

    ax.set_xticks(np.arange(len(metrics)))
    ax.set_yticks(np.arange(len(metrics)))
    ax.set_xticklabels(metrics, rotation=30, ha="right")
    ax.set_yticklabels(metrics)
    ax.set_xlabel("Affected metric")
    ax.set_ylabel("Focus metric (conditioned on improvement)")
    ax.set_title(title)

    for i in range(len(metrics)):
        for j in range(len(metrics)):
            v = mat[i, j]
            if np.isnan(v):
                text = "NA"
            else:
                text = format(v, value_fmt)
            ax.text(j, i, text, ha="center", va="center", fontsize=9)

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label(colorbar_label)

    fig.tight_layout()
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
    n = len(metrics)
    fig, axes = plt.subplots(n, 1, figsize=(max(12, 2.9 * max(1, n - 1)), 5.0 * n), sharey=False)

    if n == 1:
        axes = np.array([axes])

    for i, focus in enumerate(metrics):
        ax = axes[i]
        sub = focus_df[focus_df["focus_metric"] == focus]
        affected_metrics = list(metrics)

        groups = []
        labels = []
        positions = []
        colors = []
        pos = 1.0
        gap_between_metrics = 0.2 # was origonally 1.0, but reduced to fit more metrics without excessive whitespace.

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

            groups.extend([improved_vals, not_improved_vals])
            labels.extend([
                f"{affected}\n{focus} improved",
                f"{affected}\n{focus} not improved",
            ])
            positions.extend([pos, pos + 0.8])
            colors.extend(["#2F7D32", "#C62828"])
            pos += 0.8 + gap_between_metrics

        bp = ax.boxplot(
            groups,
            positions=positions,
            labels=labels,
            patch_artist=True,
            widths=0.62,
            showfliers=False,
        )

        for patch, color in zip(bp["boxes"], colors):
            patch.set_facecolor(color)
            patch.set_alpha(0.45)

        for median in bp["medians"]:
            median.set_color("#222222")
            median.set_linewidth(1.8)

        ax.axhline(0.0, color="#555555", linewidth=1.0, linestyle="--")
        ax.set_ylabel(f"focus: {focus}\nrelative delta")
        ax.tick_params(axis="x", labelrotation=22)
        ax.grid(axis="y", alpha=0.25)
        

    fig.suptitle(
        "Relative Effect Distributions per Focus Metric\n(Non-focus Metrics in Each Subplot)",
        fontsize=15,
        y=0.995,
    )
    fig.tight_layout(rect=[0, 0, 1, 0.988])
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
    n = len(metrics)
    fig, axes = plt.subplots(n, 1, figsize=(max(12, 2.9 * max(1, n - 1)), 4.6 * n), sharey=False)

    if n == 1:
        axes = np.array([axes])

    for i, focus in enumerate(metrics):
        ax = axes[i]
        sub = focus_df[
            (focus_df["focus_metric"] == focus)
            & (focus_df["focus_improved"] == focus_improved_value)
        ]

        groups = []
        labels = []

        for metric in metrics:
            col = f"affected_rel_delta_{metric}"
            vals = sub[col].to_numpy(dtype=float)
            vals = vals[np.isfinite(vals)]
            if len(vals) > 0:
                lo = np.percentile(vals, 2.5)
                hi = np.percentile(vals, 97.5)
                vals = np.clip(vals, lo, hi)

            groups.append(vals)
            labels.append(metric)

        bp = ax.boxplot(
            groups,
            labels=labels,
            patch_artist=True,
            widths=0.6,
            showfliers=False,
        )

        for patch in bp["boxes"]:
            patch.set_facecolor("#2F7D32" if focus_improved_value else "#C62828")
            patch.set_alpha(0.45)

        for median in bp["medians"]:
            median.set_color("#222222")
            median.set_linewidth(1.8)

        ax.axhline(0.0, color="#555555", linewidth=1.0, linestyle="--")
        ax.set_ylabel(f"focus: {focus}\nrelative delta")
        ax.tick_params(axis="x", labelrotation=22)
        ax.grid(axis="y", alpha=0.25)
        

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
    fig.tight_layout(rect=[0, 0, 1, 0.988])
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
    labels = []
    positions = []
    colors = []
    pos = 1.0
    gap_between_metrics = 1.0

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
        labels.extend([f"{metric}\n{focus_metric} improved", f"{metric}\n{focus_metric} not improved"])
        positions.extend([pos, pos + 0.8])
        colors.extend(["#2F7D32", "#C62828"])
        pos += 0.8 + gap_between_metrics

    fig, ax = plt.subplots(figsize=(max(10, 2.9 * len(affected_metrics)), 6.5))
    bp = ax.boxplot(
        groups,
        positions=positions,
        labels=labels,
        patch_artist=True,
        widths=0.62,
        showfliers=False,
    )

    for patch, color in zip(bp["boxes"], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.45)

    for median in bp["medians"]:
        median.set_color("#222222")
        median.set_linewidth(1.8)

    ax.axhline(0.0, color="#555555", linewidth=1.0, linestyle="--")
    ax.set_ylabel("Relative delta (winsorized 2.5%-97.5%)")
    ax.set_title(
        f"Effect on Non-focus Metrics When {focus_metric} Improves",
        fontsize=14,
    )
    ax.tick_params(axis="x", labelrotation=22)
    ax.grid(axis="y", alpha=0.25)

    # Add a compact legend-like annotation for color meaning.
    ax.text(
        0.01,
        0.98,
        "Green: focus improved   |   Red: focus not improved",
        transform=ax.transAxes,
        va="top",
        ha="left",
        fontsize=10,
        color="#333333",
    )

    fig.tight_layout()
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
    plot_heatmap(
        matrix_df=co_improvement_df,
        metrics=metrics,
        title="P(Affected Improves | Focus Improves)",
        colorbar_label="Probability",
        output_path=output_dir / "heatmap_co_improvement.png",
        cmap="viridis",
        value_fmt=".2f",
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
