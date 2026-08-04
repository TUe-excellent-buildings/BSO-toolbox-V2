#!/bin/bash

# Parallel version of run_design_sequence.sh
# Runs all argument lines from input_args.txt as background ./main
# processes, with an optional concurrency limit via MAX_JOBS.

# Do NOTE!, each SCDP run needs to be fully separate,
# so the input argument of 'combine_data' can only be 
# used in combination with input argument -pl so it does not write to the same file.
# later with the help of BSO-toolbox-V2/code_developments_for_publications/scdp_data_combine/merge_combined_parallel_csv.sh, 
# the separate combined_data_tmp_*.csv files
# can be merged into a single combined_data.csv file for analysis and plotting.

# How to run the script in the background without keeping the terminal open:
    # 1) The command: nohup bash run_design_sequence_parallel.sh > output.log 2>&1 & 
    # (alerted for your directory structure) can be used to run this script in the
    # background, with all output logged to output.log. This allows you to start
    # the parallel runs and then disconnect from the terminal without stopping the 
    # processes. Please take note (and store) of the PID number directly stated behind the command after [1], example: 
    # (base) tessa@Brisbane:~/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN$ nohup bash /home/tessa/BSO-toolbox-V2/code_developments_for_publications/scdp_class_RUN/run_design_sequence_parallel.sh > output.log 2>&1 &
    # [1] 2168535
    # You can monitor the progress by checking the output.log file or by
    # looking at the individual log files for each run in the logs_parallel_runs directory.
    # 2) Use ps -p ...... (..... individual process number stated in the terminal after nohub command was inserted)
    # to check if a specific process is still running
    # 3) tail -f output.log to follow the log output in real time. 
    # 4) Use 'ps aux | grep run_design_sequence_parallel.sh' to retrieve the PID number if you do not know anymore.
    # You can kill the process with 'kill 2168535' (replace 2168535 with the actual PID number) if you need to stop it.

# latest PID: 4861

echo "Running parallel bash script" 
echo "for ./main with the following input arguments (parallel):"

# Define the executable
EXECUTABLE="./main"

# Define the input arguments file
INPUT_FILE="input_args.txt"

# Check if the input file exists and is readable
if [ ! -f "$INPUT_FILE" ]; then
    echo "Input file $INPUT_FILE does not exist."
    exit 1
fi

if [ ! -r "$INPUT_FILE" ]; then
    echo "Input file $INPUT_FILE is not readable."
    exit 1
fi

# Output the content of the input arguments to the terminal
CONTENT=$(<"$INPUT_FILE")
echo "The following input is defined in $INPUT_FILE:"
echo "$CONTENT"

# Create the results directory if it doesn't exist
RESULTS_DIR="./results"
mkdir -p "$RESULTS_DIR"

# Optional: limit of concurrent jobs (0 or empty means no limit)
# Example usage: MAX_JOBS=4 ./run_design_sequence_parallel.sh
MAX_JOBS=${MAX_JOBS:-3}  # Default to 3 if not set externally

# Show the lines that will be executed
echo "The $INPUT_FILE arguments will be run in parallel as follows:"
i=1
while read -r line; do
    echo "line No. $i: $line"
    i=$((i+1))
done < "$INPUT_FILE"


echo "Start executing (parallel)"

# Directory to store per-run logs
LOG_DIR="./logs_parallel_runs"
mkdir -p "$LOG_DIR"

# Collect per-run peak memory data here.
MEMORY_LOG_DIR="./logs_parallel_runs_memory"
mkdir -p "$MEMORY_LOG_DIR"

get_descendant_pids() {
    local root_pid="$1"

    if ! kill -0 "$root_pid" 2>/dev/null; then
        return 0
    fi

    if command -v pstree >/dev/null 2>&1; then
        pstree -p "$root_pid" 2>/dev/null | grep -oE '\([0-9]+\)' | tr -d '()'
    else
        echo "$root_pid"
    fi
}

count_running_main_jobs() {
    local running=0
    local pid
    for pid in "${pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            running=$((running+1))
        fi
    done
    echo "$running"
}

# Monitor peak memory usage of a process and its descendants by
# sampling every second until the root process exits. 
# The peak memory usage is logged to a file.
monitor_peak_memory() {
    local root_pid="$1"
    local memory_log_file="$2"
    local max_kb=0
    local max_pid=""
    local max_cmd=""

    sample_memory() {
        local pid_list="$1"
        local p
        for p in $pid_list; do
            [ -r "/proc/$p/status" ] || continue

            local hwm_kb
            hwm_kb=$(awk '/VmHWM:/{print $2}' "/proc/$p/status" 2>/dev/null)
            if [ -z "$hwm_kb" ]; then
                # Fallback when VmHWM is unavailable for very short-lived processes.
                hwm_kb=$(awk '/VmRSS:/{print $2}' "/proc/$p/status" 2>/dev/null)
            fi
            [ -n "$hwm_kb" ] || continue

            if [ "$hwm_kb" -gt "$max_kb" ]; then
                max_kb="$hwm_kb"
                max_pid="$p"
                max_cmd=$(tr '\0' ' ' < "/proc/$p/cmdline" 2>/dev/null)
            fi
        done
    }

    # Attempt one immediate sample before the process can exit.
    sample_memory "$root_pid $(get_descendant_pids "$root_pid")"

    while kill -0 "$root_pid" 2>/dev/null; do
        sample_memory "$root_pid $(get_descendant_pids "$root_pid")"

        sleep 1
    done

    {
        echo "root_pid=$root_pid"
        if [ "$max_kb" -gt 0 ]; then
            echo "peak_vm_hwm_kb=$max_kb"
            awk -v kb="$max_kb" 'BEGIN { printf("peak_vm_hwm_mib=%.2f\n", kb/1024) }'
            echo "peak_process_pid=$max_pid"
            echo "peak_process_cmd=$max_cmd"
        else
            echo "peak_vm_hwm_kb=unknown"
            echo "peak_process_pid=unknown"
            echo "peak_process_cmd=unknown"
        fi
    } > "$memory_log_file"
}

pids=()
monitor_pids=()
memory_log_files=()
run_designs=()
run_args=()
run_index=1

# Read input arguments from the file and launch background jobs
while IFS= read -r ARG || [ -n "$ARG" ]; do
    echo "Read line: $ARG"

    # Skip empty lines
    if [ -z "$ARG" ]; then
        echo "Empty line detected, skipping..."
        continue
    fi

    # Derive the design number from the design id (should be present)
    DESIGN=$(echo "$ARG" | grep -oP '(?<=-i )design_[^ ]+' || true)
    if [ -z "$DESIGN" ]; then
        DESIGN="run"
    fi

    # Derive the kind of run from the input ARG (should be present)
    RUNKIND=$(echo "$ARG" | grep -oP '(?<=-m )[^ ]+' || true)
    if [ -z "$RUNKIND" ]; then
        RUNKIND="unknown"
    fi

    LOG_FILE="$LOG_DIR/${DESIGN}_${RUNKIND}_$run_index.log"
    echo "Running $EXECUTABLE with arguments $ARG (background), logging to $LOG_FILE"
    # Redirect stdout and stderr of this run to its own log file
    $EXECUTABLE $ARG >"$LOG_FILE" 2>&1 &
    child_pid="$!"
    pids+=("$child_pid")

    MEMORY_LOG_FILE="$MEMORY_LOG_DIR/${DESIGN}_${RUNKIND}_$run_index.memory.log"
    monitor_peak_memory "$child_pid" "$MEMORY_LOG_FILE" &
    monitor_pids+=("$!")
    memory_log_files+=("$MEMORY_LOG_FILE")
    run_designs+=("$DESIGN")
    run_args+=("$ARG")

    run_index=$((run_index+1))

    # Simple concurrency control if MAX_JOBS > 0
    if [ "$MAX_JOBS" -gt 0 ]; then
        # Wait until the number of running main child jobs drops below MAX_JOBS.
        # Monitor processes are excluded from this count.
        while [ "$(count_running_main_jobs)" -ge "$MAX_JOBS" ]; do
            sleep 1
        done
    fi

done < "$INPUT_FILE"

SUMMARY_CSV="$MEMORY_LOG_DIR/summary_peak_memory.csv"
echo "run_index,design,root_pid,peak_vm_hwm_kb,peak_vm_hwm_mib,peak_process_pid,peak_process_cmd,run_args" > "$SUMMARY_CSV"

# Wait for all background runs to complete
echo "Waiting for ${#pids[@]} runs to finish..."
for i in "${!pids[@]}"; do
    pid="${pids[$i]}"
    wait "$pid" || echo "Warning: process $pid exited with non-zero status."

    monitor_pid="${monitor_pids[$i]}"
    wait "$monitor_pid" 2>/dev/null || true

    memory_log_file="${memory_log_files[$i]}"
    design="${run_designs[$i]}"
    run_arg="${run_args[$i]}"

    if [ -r "$memory_log_file" ]; then
        root_pid=$(awk -F= '/^root_pid=/{print $2}' "$memory_log_file")
        peak_kb=$(awk -F= '/^peak_vm_hwm_kb=/{print $2}' "$memory_log_file")
        peak_mib=$(awk -F= '/^peak_vm_hwm_mib=/{print $2}' "$memory_log_file")
        peak_process_pid=$(awk -F= '/^peak_process_pid=/{print $2}' "$memory_log_file")
        peak_process_cmd=$(awk -F= '/^peak_process_cmd=/{print $2}' "$memory_log_file")
    else
        root_pid="$pid"
        peak_kb="unknown"
        peak_mib=""
        peak_process_pid="unknown"
        peak_process_cmd="unknown"
    fi

    safe_peak_process_cmd=$(printf '%s' "$peak_process_cmd" | sed 's/"/""/g')
    safe_run_arg=$(printf '%s' "$run_arg" | sed 's/"/""/g')

    csv_run_index=$((i+1))
    echo "$csv_run_index,$design,$root_pid,$peak_kb,$peak_mib,$peak_process_pid,\"$safe_peak_process_cmd\",\"$safe_run_arg\"" >> "$SUMMARY_CSV"

    echo "Peak memory log written to ${memory_log_files[$i]}"
done

echo "Peak-memory summary written to $SUMMARY_CSV"
echo "Summary (run_index, design, peak_vm_hwm_kb, peak_vm_hwm_mib):"
awk -F, 'NR==1 {next} {printf "%s, %s, %s, %s\\n", $1, $2, $4, $5}' "$SUMMARY_CSV"


echo "Parallel bash script execution completed."