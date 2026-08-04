#!/bin/bash

# Script description and run instructions:
# This script runs the two parallel run_design_sequence_parallel.sh scripts in sequence, one after the other.
# to run this script in the background, use: nohup bash run_both_design_sequences.sh > output_both_runs.log 2>&1 & wrapper_pid=$!
# store the latest PID in this file for easy reference and to be able to kill it if needed.
# PID: 3178588

# Kill instructions for the wrapper process and all its child processes:
# 1) Check wrapper PID and its process group ID (PGID)
# ps -o pid=,ppid=,pgid=,cmd= -p 2681868
# 2) Graceful stop for whole group (wrapper + children), with substituting 2681868 with the current PID:
# kill -TERM -- -2681868
# 3) If anything is still alive after a few seconds, force kill the group
# pkill -KILL -g 2681868


# Actual script:
# Stop immediately if:
# - a command fails (-e)
# - an unset variable is used (-u)
# - any command in a pipeline fails (-o pipefail)
set -euo pipefail

# Absolute paths to the two run folders.
# The script runs folder 2 first, then folder 3.
RUN1_DIR="/home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_run"
RUN2_DIR="/home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_run_2"
RUN3_DIR="/home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_run_3"

# A function to runs one folder's parallel script in the foreground.
# Arguments:
#   1) run_dir: folder that contains run_design_sequence_parallel.sh
#   2) label: text for status messages only
run_one() 
{
    local run_dir="$1" #$1 stand for first argument of function named run_one.
    local label="$2" #$2 stand for second argument of function named run_one.

    # Console status line with wall-clock time.
    echo "[$(date '+%F %T')] Starting $label in $run_dir"

    # Use a subshell so cd only affects this block, not the parent script.
    (
        cd "$run_dir"
        # stdout/stderr are written to the folder-local output.log file.
        bash run_design_sequence_parallel.sh > output.log 2>&1
    )

    # Printed only after that run finishes successfully.
    echo "[$(date '+%F %T')] Finished $label"
}

# Run function run_one, thus the sequential execution: second run starts only after first run completes.
run_one "$RUN2_DIR" "scdp_class_run_2"
run_one "$RUN1_DIR" "scdp_class_run"
run_one "$RUN3_DIR" "scdp_class_run_3"

# Final completion message for the wrapper script.
echo "[$(date '+%F %T')] Both sequential runs completed"
