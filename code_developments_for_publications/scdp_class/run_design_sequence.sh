#!/bin/bash

echo "Running bash script" 
echo "for ./main with the following input arguments:"

# Set this nohup ./run_design_sequence.sh > terminalLog 2>&1 &
# last PID: 483024

# Define the executable
EXECUTABLE="./main"

# Define the input arguments file
INPUT_FILE="input_args.txt"

# Output the content of the input arguments to the terminal
CONTENT=$(<$INPUT_FILE)
echo "The following input is defined in $INPUT_FILE:"
echo "$CONTENT"

# Check if the input file exists and is readable
if [ ! -f "$INPUT_FILE" ]; then
    echo "Input file $INPUT_FILE does not exist."
    exit 1
fi

if [ ! -r "$INPUT_FILE" ]; then
    echo "Input file $INPUT_FILE is not readable."
    exit 1
fi

# Create the results directory if it doesn't exist
RESULTS_DIR="./results"
mkdir -p $RESULTS_DIR

echo "The $INPUT_FILE arguments will be run as followes:"
i=1
while read line; do
#read each line
echo "line No. $i: $line"
i=$((i+1))
done < $INPUT_FILE

# Directory to store per-run logs
LOG_DIR="./logs_sequential_runs"
mkdir -p "$LOG_DIR"

pids=()
run_index=1

echo "Start executing"
# Read input arguments from the file
while IFS= read -r ARG || [ -n "$ARG" ]; do
    echo "Read line: $ARG"
	
	# Check if ARG is empty
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
    echo "Running $EXECUTABLE with arguments $ARG, logging to $LOG_FILE"
    # Redirect stdout and stderr of this run to its own log file, run sequentially
    $EXECUTABLE $ARG >"$LOG_FILE" 2>&1
    run_index=$((run_index+1))
done < "$INPUT_FILE"

echo "Bash script execution completed."
