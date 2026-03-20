#!/usr/bin/env python3
import sys
import os

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MaxNLocator


def _parse_axis_labels_from_line(line: str) -> dict:
    """Parse an "axis_labels" metadata line from the data_export headers.

    Expected format (after the leading '#'):
        axis_labels key1=Label 1; key2=Label 2; ...
    """
    labels: dict = {}
    if "axis_labels" not in line:
        return labels

    # Strip leading '#' and any whitespace
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


def load_multi_axis_data(path):
    """Load multi-axis data file written by data_export::writeMultiAxisFile.

    Expected format:
    - First line starting with '#' containing column names: '# iteration <y1> <y2> ...'
    - Remaining lines: iteration, y1, y2, ... values (whitespace or tab separated)
    """
    axis_labels: dict = {}
    with open(path, "r") as f:
        header = f.readline().strip()
        if not header.startswith("#"):
            raise ValueError(f"First line of {path} does not start with '#': {header}")
        # Split and drop leading '#'
        parts = header[1:].strip().split()
        if len(parts) < 2:
            raise ValueError("Header must contain at least iteration and one y column")
        column_names = parts  # ['iteration', 'y1', 'y2', ...]

        # Optional second header line containing axis_labels metadata.
        pos = f.tell()
        second = f.readline()
        if second and second.lstrip().startswith("#") and "axis_labels" in second:
            axis_labels = _parse_axis_labels_from_line(second)
        else:
            # No metadata line; rewind so np.loadtxt sees the next line.
            f.seek(pos)

    # Use numpy to load the numeric data (skipping all '#' comment lines)
    data = np.loadtxt(path, comments="#")
    if data.ndim == 1:
        data = data.reshape(1, -1)

    x = data[:, 0]
    ys = [data[:, i] for i in range(1, data.shape[1])]
    return x, ys, column_names, axis_labels


def plot_multi_axis(path, title=None, show=True, save_path=None, start_at_zero=False):
    x, ys, names, axis_labels = load_multi_axis_data(path)

    # names[0] is 'iteration' (x label); the rest are y-series names
    x_label = names[0]
    y_labels = names[1:]
    n_series = len(ys)

    # Restore original figure size for a balanced multi-axis layout.
    fig, ax_base = plt.subplots(figsize=(9, 5.5))
    # Leave extra space on both sides for multiple y-axes
    fig.subplots_adjust(left=0.22, right=0.80)

    # Colors for the different series / axes
    colors = [
        "tab:blue",
        "tab:orange",
        "tab:green",
        "tab:red",
        "tab:purple",
        "tab:brown",
    ]

    axes = [ax_base]

    # Helper to create a left-offset axis
    def make_left_axis(offset_axes_pos):
        ax = ax_base.twinx()
        # Show left spine at a negative offset, hide right spine
        ax.spines["left"].set_position(("axes", offset_axes_pos))
        ax.spines["left"].set_visible(True)
        ax.spines["right"].set_visible(False)
        ax.yaxis.set_label_position("left")
        ax.yaxis.tick_left()
        ax.set_frame_on(True)
        ax.patch.set_visible(False)
        return ax

    # Plot each series. Behaviour:
    #   - 1 series: single left axis
    #   - 2 series: one left axis, one right axis
    #   - 3+ series: two left axes (base + offset) and remaining on the right
    for idx, y in enumerate(ys):
        if idx == 0:
            ax = ax_base  # main left axis
            label_pad = 12
        elif idx == 1:
            if n_series == 2:
                # With exactly two series, put the second axis on the right
                ax = ax_base.twinx()
                ax.spines["right"].set_visible(True)
                ax.set_frame_on(True)
                ax.patch.set_visible(False)
                axes.append(ax)
                label_pad = 12
            else:
                # For 3+ series, use a second left axis, offset to avoid overlap
                ax = make_left_axis(-0.22)
                axes.append(ax)
                label_pad = 24
        else:
            # Additional axes as twins of the base, offset horizontally on the right
            ax = ax_base.twinx()
            offset = 1.02 + 0.22 * (idx - 1)  # 1.0 = right, >1 pushes further right
            ax.spines["right"].set_position(("axes", offset))
            ax.spines["right"].set_visible(True)
            ax.set_frame_on(True)
            ax.patch.set_visible(False)
            axes.append(ax)

            # Push labels further away so they don't sit on top of each other
            label_pad = 16 + 12 * (idx - 1)

        color = colors[idx % len(colors)]
        raw_label = y_labels[idx] if idx < len(y_labels) else f"y{idx+1}"
        label = axis_labels.get(raw_label, raw_label)

        ax.plot(x, y, color=color, label=label)
        ax.set_ylabel(label, color=color)
        # Rotate labels and add padding to reduce overlap
        ax.yaxis.label.set_rotation(90)
        ax.yaxis.labelpad = label_pad
        ax.tick_params(axis="y", colors=color, labelsize=8)

    ax_base.set_xlabel(axis_labels.get(x_label, x_label))
    # Force integer ticks on the x-axis (iteration index)
    ax_base.xaxis.set_major_locator(MaxNLocator(integer=True))

    # Build a combined legend from all axes
    handles = []
    labels = []
    for ax in axes:
        h, l = ax.get_legend_handles_labels()
        handles.extend(h)
        labels.extend(l)
    if handles:
        ax_base.legend(handles, labels, loc="best")

    if title is not None:
        ax_base.set_title(title)

    # Optionally force all axes to start at zero (x and all y-axes).
    if start_at_zero:
        try:
            ax_base.set_xlim(left=0)
        except Exception:
            pass
        for ax in axes:
            try:
                _, ymax = ax.get_ylim()
                ax.set_ylim(bottom=0, top=ymax)
            except Exception:
                continue

    fig.tight_layout()

    if save_path is not None:
        fig.savefig(save_path, dpi=150)

    if show:
        plt.show()
    else:
        plt.close(fig)


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    # Positional arguments (all optional, for backwards compatibility):
    #   0: data file path
    #   1: title
    #   2: show flag  ("1" = show window, "0" = no window)
    #   3: save flag  ("1" = save PNG,   "0" = do not save)
    #   4: png path   (optional explicit PNG output path)
    #   5: start-at-zero flag ("1" = force all axes to start at 0)

    if len(argv) == 0:
        # Default to the typical multi-axis file name in the current directory
        data_file = "multiAxisData_design_316_dlsd"
    else:
        data_file = argv[0]

    title = None
    if len(argv) >= 2 and argv[1] != "":
        title = argv[1]

    # Defaults: keep previous behaviour (show window and save PNG)
    show = True
    save = True
    png_path = None

    if len(argv) >= 3:
        show = (argv[2] == "1")
    if len(argv) >= 4:
        save = (argv[3] == "1")
    if len(argv) >= 5 and argv[4] != "":
        png_path = argv[4]

    # Optional flag (CLI or environment) to start all axes at zero
    start_zero = False
    if len(argv) >= 6:
        start_zero = (argv[5] == "1")
    else:
        env_flag = os.getenv("PLOT_MULTI_AXIS_ZERO", "").lower()
        if env_flag in ("1", "true", "yes", "on"):
            start_zero = True

    if not os.path.isfile(data_file):
        print(f"Error: data file '{data_file}' not found.")
        sys.exit(1)

    # If saving is requested but no explicit PNG path is given, save next to data file
    if save and png_path is None:
        png_path = os.path.splitext(data_file)[0] + "_matplotlib.png"

    plot_multi_axis(
        data_file,
        title=title,
        show=show,
        save_path=png_path if save else None,
        start_at_zero=start_zero,
    )


if __name__ == "__main__":
    main()
