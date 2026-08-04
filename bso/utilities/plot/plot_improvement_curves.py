#!/usr/bin/env python3
"""Per-objective improvement curves for SCDP runs.

Expected input file format (written by data_export::writeImprovementCurvesFile):

- First line:  "# iteration sd_improvement_percent bp_improvement_percent dl_ADFbuilding_improvement_percent [dl_score_improvement_percent]"
- Second line: "# kneePointIteration <index> axis_labels ..."
- Remaining lines: tab-separated columns
    iteration  sd_impr  bp_impr  dl_ADF_impr  [dl_score_impr]  (all in %)

The script plots improvement [%] vs iteration for each objective and can
optionally mark the knee-point iteration with a vertical line.

It supports both three-objective (sd, bp, dl_ADFbuilding) and four-objective
(including dl_score = BS EN 17037-2018 score) files. Any additional
"*_improvement_percent" columns will automatically be plotted.
"""

import os
import sys
from typing import Tuple, List

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator


def _parse_axis_labels_from_line(line: str) -> dict:
    """Parse an "axis_labels" metadata line from the data_export headers.

    Expected format (after the leading '#'):
        kneePointIteration <n> axis_labels key1=Label 1; key2=Label 2; ...
    """
    labels: dict = {}
    if "axis_labels" not in line:
        return labels

    text = line.lstrip("#").strip()
    idx = text.find("axis_labels")
    if idx == -1:
        return labels
    meta = text[idx + len("axis_labels") :].strip()
    for part in meta.split(";"):
        part = part.strip()
        if not part or "=" not in part:
            continue
        key, val = part.split("=", 1)
        labels[key.strip()] = val.strip()
    return labels


def _series_label_from_header(raw: str, axis_labels: dict) -> str:
    """Map a header like 'sd_improvement_percent' to a nice legend label.

    Uses the base key (e.g. 'sd') with the label coming from the
    axis_labels metadata written by data_export (mAxisLabels).
    """
    base = raw
    if raw.endswith("_improvement_percent"):
        base = raw[: -len("_improvement_percent")]

    if base in axis_labels:
        return axis_labels[base]

    # Fallback: keep previous short behaviour if no metadata was present.
    return raw.split("_")[0] if "_" in raw else raw


def load_improvement_data(path: str) -> Tuple[np.ndarray, List[np.ndarray], List[str], int, dict]:
    """Load improvement data, axis labels and knee metadata from a text file.

    The function is agnostic to the number of improvement columns: it will
    load all columns after "iteration" as separate improvement series.
    """
    with open(path, "r") as f:
        header1 = f.readline().strip()
        if not header1.startswith("#"):
            raise ValueError(f"First line of {path} must start with '#'. Got: {header1}")
        names = header1[1:].strip().split()
        # names[0] should be 'iteration'; the remaining entries are the
        # improvement series (e.g. sd_improvement_percent, bp_improvement_percent,
        # dl_ADFbuilding_improvement_percent, dl_score_improvement_percent, ...).
        if len(names) < 2:
            raise ValueError("Header must contain at least iteration and one improvement column")

        header2 = f.readline().strip()
        knee_iter = -1
        axis_labels: dict = {}
        if header2.startswith("#"):
            axis_labels = _parse_axis_labels_from_line(header2)
            if "kneePointIteration" in header2:
                parts = header2[1:].strip().split()
                if len(parts) >= 2 and parts[0] == "kneePointIteration":
                    try:
                        knee_iter = int(parts[1])
                    except ValueError:
                        knee_iter = -1
        else:
            # Second line is not kneePointIteration; rewind so np.loadtxt sees it
            f.seek(0)

    data = np.loadtxt(path, comments="#")
    if data.ndim == 1:
        data = data.reshape(1, -1)

    iteration = data[:, 0]
    # All remaining columns are improvement series.
    curves: List[np.ndarray] = []
    for col in range(1, data.shape[1]):
        curves.append(data[:, col])

    return iteration, curves, names, knee_iter, axis_labels


def plot_improvement_curves(
    path: str,
    title: str = None,
    show: bool = True,
    save_path: str = None,
    show_knee: bool = False,
    include_fourth_objective: bool = False
) -> None:
    it, curves, names, knee_iter, axis_labels = load_improvement_data(path)

    # The default view is the original three-objective plot. When the
    # optional fourth objective is enabled, plot every loaded series.
    if not include_fourth_objective:
        curves = curves[:3]
        names = names[:4]  # keep 'iteration' + first three objectives

    # Use the original figure size that keeps the plot balanced.
    fig, ax = plt.subplots(figsize=(8.0, 4.8))

    # Provide a small palette; if there are more series than colors,
    # they will wrap around.
    colors = ["tab:blue", "tab:orange", "tab:green", "tab:red"]

    for idx, y in enumerate(curves):
        # Map raw header names to descriptive series labels using
        # the axis-label metadata written by data_export (mAxisLabels).
        raw_label = names[idx + 1]  # skip 'iteration'
        label = _series_label_from_header(raw_label, axis_labels)
        ax.plot(it, y, marker="o", color=colors[idx % len(colors)], label=label)

    # Optionally highlight the knee-point iteration with a vertical line.
    if show_knee and 0 <= knee_iter < len(it):
        ax.axvline(x=knee_iter, color="red", linestyle="--", linewidth=1.2, label="Knee iteration")

    ax.set_xlabel(axis_labels.get("iteration", "Iteration"))
    # Make sure iteration axis shows whole numbers only and that the
    # x-axis starts at 0 (first iteration)
    ax.xaxis.set_major_locator(MaxNLocator(integer=True))
    if len(it) > 0:
        ax.set_xlim(left=0, right=it.max())

    # Clarify that improvements are measured relative to the initial design
    ax.set_ylabel("Improvement vs initial design [%]")
    ax.grid(True, linestyle=":", linewidth=0.5)
    if title:
        ax.set_title(title)

    ax.legend(loc="best")

    fig.tight_layout()

    if save_path is not None:
        fig.savefig(save_path, dpi=150)

    if show:
        plt.show()
    else:
        plt.close(fig)


def main(argv=None) -> None:
    if argv is None:
        argv = sys.argv[1:]

    # Positional arguments (all optional):
    #   0: data file path
    #   1: title
    #   2: show flag  ("1" = show window, "0" = no window)
    #   3: save flag   ("1" = save PNG,   "0" = do not save)
    #   4: png path    (optional explicit PNG output path)
    #   5: knee flag   ("1" = show knee,  "0" = hide knee; default 0)
    #   6: include fourth objective (i.e., dl_score) flag ("1" = include, "0" = exclude; default 0)

    if len(argv) == 0:
        data_file = "improvementCurvesData.txt"
    else:
        data_file = argv[0]

    title = None
    if len(argv) >= 2 and argv[1] != "":
        title = argv[1]

    show = True
    save = True
    png_path = None
    show_knee = False
    include_fourth_objective = False
    if len(argv) >= 3:
        show = (argv[2] == "1")
    if len(argv) >= 4:
        save = (argv[3] == "1")
    if len(argv) >= 5 and argv[4] != "":
        png_path = argv[4]
    if len(argv) >= 6:
        show_knee = (argv[5] == "1")
    if len(argv) >= 7:
        include_fourth_objective = (argv[6] == "1")

    if not os.path.isfile(data_file):
        print(f"Error: data file '{data_file}' not found.")
        sys.exit(1)

    if save and png_path is None:
        png_path = os.path.splitext(data_file)[0] + "_improvement.png"

    plot_improvement_curves(
        data_file,
        title=title,
        show=show,
        save_path=png_path if save else None,
        show_knee=show_knee,
        include_fourth_objective=include_fourth_objective,
    )


if __name__ == "__main__":
    main()
