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
MAX_JOBS=${MAX_JOBS:-1}  # Default to 2 if not set externally

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

pids=()
run_index=1

# Read input arguments from the file and launch background jobs
while IFS= read -r ARG || [ -n "$ARG" ]; do
    echo "Read line: $ARG"

    # Skip empty lines
    if [ -z "$ARG" ]; then
        echo "Empty line detected, skipping..."
        continue
    fi

    # Derive a simple label from the design id (if present)
    DESIGN=$(echo "$ARG" | grep -oP '(?<=-i )design_[^ ]+' || true)
    if [ -z "$DESIGN" ]; then
        DESIGN="run"
    fi

    LOG_FILE="$LOG_DIR/${DESIGN}_$run_index.log"
    echo "Running $EXECUTABLE with arguments $ARG (background), logging to $LOG_FILE"
    # Redirect stdout and stderr of this run to its own log file
    $EXECUTABLE $ARG >"$LOG_FILE" 2>&1 &
    pids+=("$!")
    run_index=$((run_index+1))

    # Simple concurrency control if MAX_JOBS > 0
    if [ "$MAX_JOBS" -gt 0 ]; then
        # Wait until the number of background jobs drops below MAX_JOBS
        while [ "$(jobs -pr | wc -l)" -ge "$MAX_JOBS" ]; do
            sleep 1
        done
    fi

done < "$INPUT_FILE"

# Wait for all background runs to complete
echo "Waiting for ${#pids[@]} runs to finish..."
for pid in "${pids[@]}"; do
    wait "$pid" || echo "Warning: process $pid exited with non-zero status."
done


echo "Parallel bash script execution completed."