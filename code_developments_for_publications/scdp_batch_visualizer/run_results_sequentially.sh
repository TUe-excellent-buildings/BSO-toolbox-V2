#!/usr/bin/env bash
set -euo pipefail

show_help() {
    cat <<'EOF'
Usage:
  ./run_results_sequentially.sh <results_root_directory> [runner options...] [visualizer options...]

Description:
  Runs the ./main visualizer once for each immediate subdirectory inside the given
  results root directory, in sorted order.

Arguments:
  <results_root_directory>   Directory whose subdirectories will be visualized one by one

Forwarded visualizer options:
  Any extra arguments not recognized as runner options are passed directly to ./main, for example:
    --mode=colored
    --mode=plain
    --space-id=off

Runner options:
  --ask-continue              Ask after each subdirectory whether to continue
  --stop-after <N>            Stop after visualizing N subdirectories

Examples:
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --mode=colored
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --mode=plain --space-id=off
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --ask-continue
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --stop-after 5 --mode=colored

Notes:
  - Close each visualization window before the next subdirectory starts.
  - Press q in the visualization window to close it.
EOF
}

if [[ $# -lt 1 ]]; then
    show_help
    exit 1
fi

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    show_help
    exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
VISUALIZER="$SCRIPT_DIR/main"
RESULTS_ROOT="$1"
shift
ASK_CONTINUE=0
STOP_AFTER=0
VISUALIZER_ARGS=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ask-continue)
      ASK_CONTINUE=1
      shift
      ;;
    --stop-after)
      if [[ $# -lt 2 ]]; then
        echo "Missing value after --stop-after" >&2
        exit 1
      fi
      if [[ ! "$2" =~ ^[0-9]+$ ]]; then
        echo "Invalid --stop-after value: $2" >&2
        exit 1
      fi
      STOP_AFTER="$2"
      shift 2
      ;;
    *)
      VISUALIZER_ARGS+=("$1")
      shift
      ;;
  esac
done

if [[ ! -x "$VISUALIZER" ]]; then
    echo "Visualizer executable not found or not executable: $VISUALIZER" >&2
    exit 1
fi

if [[ ! -d "$RESULTS_ROOT" ]]; then
    echo "Results root directory does not exist: $RESULTS_ROOT" >&2
    exit 1
fi

mapfile -d '' SUBDIRECTORIES < <(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z)

if [[ ${#SUBDIRECTORIES[@]} -eq 0 ]]; then
    echo "No subdirectories found in: $RESULTS_ROOT" >&2
    exit 1
fi

echo "Found ${#SUBDIRECTORIES[@]} subdirectories to visualize."

INDEX=0
FAILURES=0
for subdir in "${SUBDIRECTORIES[@]}"; do
    INDEX=$((INDEX + 1))
    echo "[$INDEX/${#SUBDIRECTORIES[@]}] Visualizing: $subdir"

  if ! "$VISUALIZER" "${VISUALIZER_ARGS[@]}" "$subdir"; then
    echo "[$INDEX/${#SUBDIRECTORIES[@]}] Skipping after visualizer failure: $subdir" >&2
    FAILURES=$((FAILURES + 1))
  fi

  if [[ "$STOP_AFTER" -gt 0 && "$INDEX" -ge "$STOP_AFTER" ]]; then
    echo "Stopping early after $INDEX subdirectories due to --stop-after $STOP_AFTER."
    break
  fi

  if [[ "$ASK_CONTINUE" -eq 1 ]]; then
    read -r -p "Continue to next subdirectory? [Y/n] " answer
    case "$answer" in
      [Nn]|[Nn][Oo])
        echo "Stopping early by user request (--ask-continue)."
        break
        ;;
      *)
        ;;
    esac
  fi
done

if [[ $FAILURES -gt 0 ]]; then
  echo "Finished visualizing all subdirectories with $FAILURES failure(s)." >&2
  exit 1
fi

echo "Finished visualizing all subdirectories."
