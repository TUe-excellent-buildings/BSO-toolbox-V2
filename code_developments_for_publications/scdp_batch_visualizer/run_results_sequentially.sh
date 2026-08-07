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
  --run-type <TYPE>           Filter subdirectories by run type: all|sd|bp|dl|sdbpdl|sdbpdl_weighted (default: all)
  --parallel                  Run visualizations in parallel batches (default)
  --no-parallel               Disable parallel batches (run one by one)
  --batch-size <N>            Number of parallel visualizations per batch (default: 5)

Examples:
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --mode=colored
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --mode=plain --space-id=off
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --ask-continue
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --stop-after 5 --mode=colored
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --batch-size 5 --mode=colored
  ./run_results_sequentially.sh /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/results_sequential_run_31_03 --no-parallel --mode=colored
  ./run_results_sequentially.sh /home/tessa/Data/Results_split_delete/results --run-type dl --mode=colored
  ./run_results_sequentially.sh /home/tessa/Data/Results_split_delete/results --run-type sdbpdl_weighted

Notes:
  - In parallel mode, up to --batch-size windows can be open at once.
  - In non-parallel mode, close each visualization window before the next starts.
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
PARALLEL=1
BATCH_SIZE=5
RUN_TYPE_FILTER="all"
VISUALIZER_ARGS=()

run_type_to_regex() {
  case "$1" in
    all)
      echo ""
      ;;
    sd)
      echo '_sd_'
      ;;
    bp)
      echo '_bp_'
      ;;
    dl)
      echo '_dl_'
      ;;
    sdbpdl)
      echo '_sdbpdl_-n='
      ;;
    sdbpdl_weighted)
      echo '_sdbpdl_[0-9]'
      ;;
    *)
      return 1
      ;;
  esac
}

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
    --parallel)
      PARALLEL=1
      shift
      ;;
    --no-parallel)
      PARALLEL=0
      shift
      ;;
    --run-type)
      if [[ $# -lt 2 ]]; then
        echo "Missing value after --run-type" >&2
        exit 1
      fi
      case "$2" in
        all|sd|bp|dl|sdbpdl|sdbpdl_weighted)
          RUN_TYPE_FILTER="$2"
          ;;
        *)
          echo "Invalid --run-type value: $2 (expected one of: all, sd, bp, dl, sdbpdl, sdbpdl_weighted)" >&2
          exit 1
          ;;
      esac
      shift 2
      ;;
    --batch-size)
      if [[ $# -lt 2 ]]; then
        echo "Missing value after --batch-size" >&2
        exit 1
      fi
      if [[ ! "$2" =~ ^[0-9]+$ || "$2" -lt 1 ]]; then
        echo "Invalid --batch-size value: $2 (must be an integer >= 1)" >&2
        exit 1
      fi
      BATCH_SIZE="$2"
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

RUN_TYPE_REGEX="$(run_type_to_regex "$RUN_TYPE_FILTER")"
if [[ "$RUN_TYPE_FILTER" == "all" ]]; then
  mapfile -d '' SUBDIRECTORIES < <(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z)
else
  mapfile -d '' SUBDIRECTORIES < <(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -print0 | grep -zE "$RUN_TYPE_REGEX" | sort -z)
fi

if [[ ${#SUBDIRECTORIES[@]} -eq 0 ]]; then
    echo "No subdirectories found in: $RESULTS_ROOT for run type filter: $RUN_TYPE_FILTER" >&2
    exit 1
fi

echo "Found ${#SUBDIRECTORIES[@]} subdirectories to visualize (run type: $RUN_TYPE_FILTER)."

if [[ -z "${DISPLAY:-}" ]]; then
  echo "Warning: DISPLAY is not set. GUI windows may not appear. Start your X server and set DISPLAY (for example: export DISPLAY=localhost:10.0)." >&2
elif command -v xdpyinfo >/dev/null 2>&1; then
  if ! xdpyinfo >/dev/null 2>&1; then
    echo "Warning: Cannot connect to X server via DISPLAY=$DISPLAY. Ensure X server (for example VcXsrv) is running and X11 forwarding is enabled." >&2
  fi
else
  echo "Warning: xdpyinfo is not installed, so X11 connectivity could not be verified." >&2
fi

if [[ "$PARALLEL" -eq 0 ]]; then
  BATCH_SIZE=1
fi

echo "Mode: $([[ "$PARALLEL" -eq 1 ]] && echo "parallel" || echo "sequential") (batch size: $BATCH_SIZE)"

INDEX=0
FAILURES=0
TOTAL_TO_RUN=${#SUBDIRECTORIES[@]}
if [[ "$STOP_AFTER" -gt 0 && "$STOP_AFTER" -lt "$TOTAL_TO_RUN" ]]; then
  TOTAL_TO_RUN="$STOP_AFTER"
fi

while [[ "$INDEX" -lt "$TOTAL_TO_RUN" ]]; do
  batch_start=$((INDEX + 1))
  batch_end=$((INDEX + BATCH_SIZE))
  if [[ "$batch_end" -gt "$TOTAL_TO_RUN" ]]; then
    batch_end="$TOTAL_TO_RUN"
  fi

  echo "Launching batch [$batch_start-$batch_end/$TOTAL_TO_RUN]"

  PIDS=()
  LABELS=()
  for ((i = INDEX; i < batch_end; i++)); do
    subdir="${SUBDIRECTORIES[$i]}"
    current=$((i + 1))
    echo "[$current/$TOTAL_TO_RUN] Visualizing: $subdir"
    "$VISUALIZER" "${VISUALIZER_ARGS[@]}" "$subdir" &
    PIDS+=("$!")
    LABELS+=("[$current/$TOTAL_TO_RUN] $subdir")
  done

  for ((j = 0; j < ${#PIDS[@]}; j++)); do
    if ! wait "${PIDS[$j]}"; then
      echo "${LABELS[$j]} failed." >&2
      FAILURES=$((FAILURES + 1))
    fi
  done

  INDEX="$batch_end"

  if [[ "$INDEX" -ge "$TOTAL_TO_RUN" ]]; then
    if [[ "$STOP_AFTER" -gt 0 && "$STOP_AFTER" -eq "$TOTAL_TO_RUN" ]]; then
      echo "Stopping early after $INDEX subdirectories due to --stop-after $STOP_AFTER."
    fi
    break
  fi

  if [[ "$ASK_CONTINUE" -eq 1 ]]; then
    read -r -p "Continue to next batch? [Y/n] " answer
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
