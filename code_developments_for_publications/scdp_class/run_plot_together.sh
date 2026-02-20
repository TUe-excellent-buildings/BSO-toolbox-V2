#!/bin/bash

echo "Running bash script" 
echo "for ./scdp_plane_adjustments plot together the sd, bp ,sdbp, and bpsd runs:"

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
CONTENT=$(<$INPUT_FILE)
echo "The following input is defined in $INPUT_FILE:"
echo "$CONTENT"

# Create the results directory if it doesn't exist
RESULTS_DIR="./results"
mkdir -p $RESULTS_DIR

# Define the executable
EXECUTABLE="./scdp_plane_adjustments"

echo "Start executing"
# Read input arguments from the file
count=0  # Initialize count variable
while IFS= read -r ARG || [ -n "$ARG" ]; do
    echo "Read line: $ARG"

    # Check if ARG is empty
    if [ -z "$ARG" ]; then
        echo "Empty line detected, skipping..."
        continue
    fi

    echo "Running $EXECUTABLE with arguments $ARG"

    $EXECUTABLE $ARG

    count=$((count+1))

    # Extract the design_* section from ARG
    DESIGN=$(echo $ARG | grep -oP '(?<=-i )design_[^ ]+')

    # Every four runs (i.e. sd, bp, weighted, sdbp, and bpsd done per design), generate the plot:
    if (( count % 3 == 0 )); then
        echo "Generating plot after $count runs"
        
        # Output file for combined data
        COMBINED_DATA_FILE="combined_vectorData_${DESIGN}.txt"
        
        # Clear the combined data file if it exists
        > $COMBINED_DATA_FILE
        
        # Define the possible extensions
        EXTENSIONS=("bpsd" "sdbp" "weighted" "bp" "sd")
        
        # Array to store the order of extensions processed
        PROCESSED_EXTENSIONS=()
        
        # Loop through each results directory that starts with the extracted design name
        for dir in $RESULTS_DIR/${DESIGN}_*; do
            echo "Processing directory: $dir"
            if [ -d "$dir" ]; then
                for ext in "${EXTENSIONS[@]}"; do
                    VECTOR_DATA_FILE=$(find "$dir" -name "vectorData_design_*_$ext")
                    if [ -f "$VECTOR_DATA_FILE" ]; then
                        echo "Found file: $VECTOR_DATA_FILE"
                        # Extract the directory name and file extension to use as a label
                        LABEL=$(basename "$dir")_$ext
                        echo $LABEL
                        # Add a comment line with the label
                        echo "# $LABEL" >> $COMBINED_DATA_FILE
                        # Append the data to the combined file
                        cat "$VECTOR_DATA_FILE" >> $COMBINED_DATA_FILE
                        # Add a blank line to separate datasets
                        echo "" >> $COMBINED_DATA_FILE
                        echo "" >> $COMBINED_DATA_FILE
                        # Store the processed extension
                        PROCESSED_EXTENSIONS+=("$ext")
                    else
                        echo "No file found for extension: $ext in directory: $dir"
                    fi
                done
            else
                echo "No directory found for: $dir"
            fi
        done
        
        echo "Combined data written to $COMBINED_DATA_FILE"
        echo "Start plotting combined data with GNUPlot"
 
        # Plot the combined data using gnuplot with different colors and a legend for each data set
        gnuplot -persist <<-EOFMarker
            set terminal png
            set output 'combined_plot_${DESIGN}.png'
            set title 'Iteration plot overview of ${DESIGN} for bp, sd, and weighted'
            set xlabel 'sd'
            set ylabel 'bp'
            # Highlight the first value of the first dataset
            first_value_x = system("awk 'NR==3 {print \$1}' $COMBINED_DATA_FILE")
            first_value_y = system("awk 'NR==3 {print \$2}' $COMBINED_DATA_FILE")
            set label "Initial design" at first_value_x, first_value_y point pointtype 7 lc rgb "red"
            plot '$COMBINED_DATA_FILE' index 0 using 1:2:3:4 with vectors filled head title '${PROCESSED_EXTENSIONS[0]}',\
                 '$COMBINED_DATA_FILE' index 1 using 1:2:3:4 with vectors filled head title '${PROCESSED_EXTENSIONS[1]}',\
                 '$COMBINED_DATA_FILE' index 2 using 1:2:3:4 with vectors filled head title '${PROCESSED_EXTENSIONS[2]}'
            set terminal x11
            replot
EOFMarker
    fi
    
done < "$INPUT_FILE"

echo "Bash script execution completed."
