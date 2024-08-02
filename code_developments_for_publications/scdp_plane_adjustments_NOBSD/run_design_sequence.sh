#!/bin/bash

echo "Running bash scribt" 
echo "for ./scdp_plane_adjustments with the following input arguments:"

# Define the executable
EXECUTABLE="./scdp_plane_adjustments"

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

echo "Start executing"
# Read input arguments from the file
while IFS= read -r ARG || [ -n "$ARG" ]; do
    echo "Read line: $ARG"
	
	# Check if ARG is empty
	if [ -z "$ARG" ]; then
        echo "Empty line detected, skipping..."
        continue
	fi
	
	# Get the current date and time
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    
    # Create a sanitized version of the input arguments for the directory name
    SANITIZED_ARG=$(echo "$ARG" | tr -c '[:alnum:]' '_')
    
    # Create a subdirectory for this run
    RUN_DIR="$RESULTS_DIR/${SANITIZED_ARG}_$TIMESTAMP"
    mkdir -p $RUN_DIR
    
    echo "Running $EXECUTABLE with arguments $ARG"
    
    # Run the executable and capture the list of generated files
    BEFORE=$(ls)
    $EXECUTABLE $ARG
    AFTER=$(ls)
    
    # Identify newly generated files
    NEW_FILES=$(comm -13 <(echo "$BEFORE") <(echo "$AFTER"))
    
    # Move newly generated files to the subdirectory
    for FILE in $NEW_FILES; do
        if [ -f "$FILE" ]; then
            mv "$FILE" "$RUN_DIR/"
        fi
    done
	
done < "$INPUT_FILE"

echo "Bash script execution completed."
