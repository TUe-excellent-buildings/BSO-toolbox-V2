#!/usr/bin/env python3
"""3D objective-space plot for SCDP runs.

Expected input file format (written by data_export::writeObjective3DFile):

- First line:  "# iteration sd bp dl_ADFbuilding"
- Second line: "# kneePointIteration <index>"
- Remaining lines: tab-separated columns
      iteration  sd  bp  dl_ADFbuilding

The script plots the path of the run in (sd, bp, dl_ADFbuilding) space and
highlights both the first iteration and the knee-point iteration.
"""

import os
import sys
from typing import Tuple, List

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401  (needed for 3D projection)


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



def load_objective_3d_data(path: str) -> Tuple[np.ndarray, List[np.ndarray], List[str], int, dict]:
    """Load objective data, axis labels and knee metadata from a text file."""
    with open(path, "r") as f:
        header1 = f.readline().strip()
        if not header1.startswith("#"):
            raise ValueError(f"First line of {path} must start with '#'. Got: {header1}")
        names = header1[1:].strip().split()  # ['iteration', 'sd', 'bp', 'dl_ADFbuilding']
        if len(names) < 4:
            raise ValueError("Header must contain at least iteration, sd, bp, dl_ADFbuilding")

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

    # Load numeric data, skipping comment lines
    data = np.loadtxt(path, comments="#")
    if data.ndim == 1:
        data = data.reshape(1, -1)

    iteration = data[:, 0]
    sd = data[:, 1]
    bp = data[:, 2]
    dl_adf = data[:, 3]

    return iteration, [sd, bp, dl_adf], names, knee_iter, axis_labels


def plot_objective_space_3d(path: str, title: str = None, show: bool = True, save_path: str = None) -> None:
    it, (sd, bp, dl_adf), names, knee_iter, axis_labels = load_objective_3d_data(path)

    # Restore original figure size for a balanced 3D plot.
    fig = plt.figure(figsize=(7.5, 6.0))
    ax = fig.add_subplot(111, projection="3d")

    # Plot the full path as a polyline for exact connections.
    # Arrows in 3D with matplotlib often introduce confusing vertical
    # artefacts due to internal autoscaling, so we use a clean line
    # with markers instead.
    ax.plot(sd, bp, dl_adf, color="C0", marker="o", markersize=4, linewidth=1.2, label="path")

    # Highlight first iteration (start) if available
    if len(sd) > 0:
        ax.scatter(sd[0], bp[0], dl_adf[0],
                   color="black", s=70, marker="o", label="start (iter 0)")

    # Highlight knee point if available
    if 0 <= knee_iter < len(sd):
        ax.scatter(sd[knee_iter], bp[knee_iter], dl_adf[knee_iter],
                   color="red", s=80, marker="^", label="knee point")

    ax.set_xlabel(axis_labels.get(names[1], names[1]))  # 'sd'
    ax.set_ylabel(axis_labels.get(names[2], names[2]))  # 'bp'
    ax.set_zlabel(axis_labels.get(names[3], names[3]))  # 'dl_ADFbuilding'

    # Invert sd-axis so smaller values are shown on the right.
    ax.invert_xaxis()

    if title:
        ax.set_title(title)

    # Put legend outside for clarity
    ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.0))

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
    #   3: save flag  ("1" = save PNG,   "0" = do not save)
    #   4: png path   (optional explicit PNG output path)

    if len(argv) == 0:
        data_file = "objective3DData.txt"
    else:
        data_file = argv[0]

    title = None
    if len(argv) >= 2 and argv[1] != "":
        title = argv[1]

    show = True
    save = True
    png_path = None

    if len(argv) >= 3:
        show = (argv[2] == "1")
    if len(argv) >= 4:
        save = (argv[3] == "1")
    if len(argv) >= 5 and argv[4] != "":
        png_path = argv[4]

    if not os.path.isfile(data_file):
        print(f"Error: data file '{data_file}' not found.")
        sys.exit(1)

    if save and png_path is None:
        png_path = os.path.splitext(data_file)[0] + "_3d.png"

    plot_objective_space_3d(
        data_file,
        title=title,
        show=show,
        save_path=png_path if save else None,
    )


if __name__ == "__main__":
    main()
