#!/usr/bin/env python3
"""Create per-design 3D objective plots comparing two modification sets.

For each matched design key found in both roots, this script reads:
  <run_dir>/combined_data_parallel.csv

It extracts the iteration table rows (0, 1, 2, ...) for the columns:
  sd, bp, dl_ADFbuilding_inverse

Then it generates one 3D scatter plot per design key with:
  - one color for modify runs
  - one color for split/delete runs

No path line is plotted; only dots are shown.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
import re
from typing import Dict, Iterable, List, Optional, Tuple

import matplotlib.pyplot as plt
import numpy as np


AXIS_LABELS = {
    "sd": "Total strain energy [Nmm]",
    "bp": "Heating and cooling energy [kWh]",
    "dl_ADFbuilding_inverse": "Average daylight factor [1/%]",
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
        "--dpi",
        type=int,
        default=150,
        help="PNG resolution.",
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


def load_iteration_points(csv_path: Path) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Load sd, bp, dl_ADFbuilding_inverse points from iteration rows only."""
    with csv_path.open("r", newline="") as f:
        rows = list(csv.reader(f))

    header_idx: Optional[int] = None
    sd_col: Optional[int] = None
    bp_col: Optional[int] = None
    dl_col: Optional[int] = None

    for i, row in enumerate(rows):
        normalized = [c.strip() for c in row]
        if "sd" in normalized and "bp" in normalized and "dl_ADFbuilding_inverse" in normalized:
            header_idx = i
            sd_col = normalized.index("sd")
            bp_col = normalized.index("bp")
            dl_col = normalized.index("dl_ADFbuilding_inverse")
            break

    if header_idx is None or sd_col is None or bp_col is None or dl_col is None:
        raise ValueError(f"Could not find objective header row in {csv_path}")

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

        sd_vals.append(sd)
        bp_vals.append(bp)
        dl_vals.append(dl)

    if not sd_vals:
        raise ValueError(f"No iteration objective points found in {csv_path}")

    return np.array(sd_vals), np.array(bp_vals), np.array(dl_vals)


def load_run_data(run_dir: Path, csv_name: str) -> RunData:
    csv_path = run_dir / csv_name
    sd, bp, dl_adf_inv = load_iteration_points(csv_path)
    return RunData(
        design_key=design_key_from_run_dir_name(run_dir.name),
        run_dir=run_dir,
        csv_path=csv_path,
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


def plot_design_comparison(
    design_key: str,
    modify_data: RunData,
    split_delete_data: RunData,
    output_file: Path,
    dpi: int,
) -> None:
    fig = plt.figure(figsize=(7.5, 6.0))
    ax = fig.add_subplot(111, projection="3d")

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
        marker="o",
        s=26,
        alpha=0.85,
        label=f"split/delete (n={len(split_delete_data.sd)})",
    )

    ax.set_xlabel(AXIS_LABELS["sd"])
    ax.set_ylabel(AXIS_LABELS["bp"])
    ax.set_zlabel(AXIS_LABELS["dl_ADFbuilding_inverse"])

    # Keep the same sd-axis orientation as the existing objective 3D plot utility.
    ax.invert_xaxis()

    ax.set_title(f"Objective 3D comparison: {design_key}")
    ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.0))

    fig.tight_layout()
    fig.savefig(output_file, dpi=dpi)
    plt.close(fig)


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
) -> None:
    fig = plt.figure(figsize=(7.5, 6.0))
    ax = fig.add_subplot(111, projection="3d")

    # Color shows modification set; marker shows objective variant.
    for objective in OBJECTIVE_ORDER:
        marker = OBJECTIVE_MARKERS[objective]
        label_obj = OBJECTIVE_LABELS[objective]

        if objective in modify_runs:
            d = modify_runs[objective]
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

        if objective in split_runs:
            d = split_runs[objective]
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

    ax.set_xlabel(AXIS_LABELS["sd"])
    ax.set_ylabel(AXIS_LABELS["bp"])
    ax.set_zlabel(AXIS_LABELS["dl_ADFbuilding_inverse"])
    ax.invert_xaxis()

    ax.set_title(f"Objective 3D comparison (all objectives): {base_design}")
    ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.0), fontsize=8)

    fig.tight_layout()
    fig.savefig(output_file, dpi=dpi)
    plt.close(fig)


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
                )
                generated_combined += 1
                print(f"[OK] {base_design} (combined) -> {out_path}")
            except Exception as exc:
                failed.append((f"{base_design} (combined)", str(exc)))
                print(f"[FAIL] {base_design} (combined): {exc}")

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
