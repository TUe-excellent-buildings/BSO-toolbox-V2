#!/usr/bin/env bash
set -euo pipefail

# Merge all combined_data_parallel.csv files produced by parallel runs into
# one combined_data.csv style file (column-wise, like sequential mode).
#
# Usage:
#   ./merge_combined_parallel_csv.sh <results_root_dir> [output_csv] [input_pattern] [run_types]
#
# Example:
#   ./merge_combined_parallel_csv.sh \
#     /home/tessa/BSO-toolbox-V2/code_developments_for_publications/Data/Results_modify_non_delete_fragments
#
# Notes:
# - The script searches recursively in all subdirectories of results_root_dir.
# - input_pattern defaults to combined_data_parallel.csv.
# - run_types defaults to all and can be a comma-separated list:
#   sd,bp,dl,sdbpdl,sdbpdl_weighted

RESULTS_ROOT="${1:-}"
OUTPUT_CSV="${2:-}"
INPUT_PATTERN="${3:-combined_data_parallel.csv}"
RUN_TYPES_ARG="${4:-all}"

if [[ -z "$RESULTS_ROOT" ]]; then
    echo "Usage: $0 <results_root_dir> [output_csv] [input_pattern] [run_types]"
    exit 1
fi

if [[ ! -d "$RESULTS_ROOT" ]]; then
    echo "Error: results root directory does not exist: $RESULTS_ROOT"
    exit 1
fi

if [[ -z "$OUTPUT_CSV" ]]; then
    OUTPUT_CSV="$RESULTS_ROOT/combined_data.csv"
fi

mapfile -t FOUND_FILES < <(find "$RESULTS_ROOT" -type f -name "$INPUT_PATTERN" | sort)

if [[ ${#FOUND_FILES[@]} -eq 0 ]]; then
    echo "Error: no files matching '$INPUT_PATTERN' found under: $RESULTS_ROOT"
    exit 1
fi

# Build a sortable key from parent directory name:
#   design_<id>_<runKind>_..._-n=...
# Desired runKind order per design:
#   sd -> bp -> dl -> sdbpdl -> sdbpdl_weighted
# A weighted sdbpdl run is detected when there is non-empty text between
# "_sdbpdl_" and "_-n=", e.g. "_sdbpdl_0.333-0.333-0.333_-n=...".
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT
SORTED_LIST_FILE="$TMP_DIR/sort_keys.txt"
SORTED_ROWS_FILE="$TMP_DIR/sorted_rows.txt"

detect_run_rank() {
    local run_dir_basename="$1"

    if [[ "$run_dir_basename" == *_sd_-n=* ]]; then
        echo "1"
    elif [[ "$run_dir_basename" == *_bp_-n=* ]]; then
        echo "2"
    elif [[ "$run_dir_basename" == *_dl_-n=* ]]; then
        echo "3"
    elif [[ "$run_dir_basename" == *_sdbpdl_-n=* ]]; then
        echo "4" # sdbpdl (non-weighted)
    elif [[ "$run_dir_basename" == *_sdbpdl_*_-n=* ]]; then
        echo "5" # sdbpdl_weighted
    else
        echo "99"
    fi
}

run_type_from_rank() {
    local run_rank="$1"
    case "$run_rank" in
        1) echo "sd" ;;
        2) echo "bp" ;;
        3) echo "dl" ;;
        4) echo "sdbpdl" ;;
        5) echo "sdbpdl_weighted" ;;
        *) echo "unknown" ;;
    esac
}

build_typed_output_path() {
    local base_output="$1"
    local run_type="$2"

    if [[ "$base_output" == *.csv ]]; then
        echo "${base_output%.csv}_${run_type}.csv"
    else
        echo "${base_output}_${run_type}.csv"
    fi
}

for FILE in "${FOUND_FILES[@]}"; do
    RUN_DIR_BASENAME="$(basename "$(dirname "$FILE")")"

    DESIGN_ID="999999"
    if [[ "$RUN_DIR_BASENAME" =~ ^design_([0-9]+)_ ]]; then
        DESIGN_ID="${BASH_REMATCH[1]}"
    fi

    RUN_RANK="$(detect_run_rank "$RUN_DIR_BASENAME")"
    RUN_TYPE="$(run_type_from_rank "$RUN_RANK")"

    # Keep deterministic order for duplicates by using full directory name.
    printf "%09d|%02d|%s|%s|%s\n" "$DESIGN_ID" "$RUN_RANK" "$RUN_DIR_BASENAME" "$RUN_TYPE" "$FILE" >> "$SORTED_LIST_FILE"
done

sort -t '|' -k1,1n -k2,2n -k3,3 "$SORTED_LIST_FILE" > "$SORTED_ROWS_FILE"
mapfile -t FILES < <(cut -d'|' -f5- "$SORTED_ROWS_FILE")

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "Error: no files matching '$INPUT_PATTERN' found under: $RESULTS_ROOT"
    exit 1
fi

prepare_input_file() {
    local src_file="$1"
    local run_dir_basename
    run_dir_basename="$(basename "$(dirname "$src_file")")"

    # Weighted runs are encoded as: ..._sdbpdl_<weights>_-n=...
    if [[ "$run_dir_basename" == *_sdbpdl_*_-n=* && "$run_dir_basename" != *_sdbpdl_-n=* ]]; then
        local dst_file
        dst_file="$TMP_DIR/prepared_$(basename "$(dirname "$src_file")")_$(basename "$src_file")"
        awk '
            NR == 1 {
                sub(/_sdbpdl/, "_sdbpdl_weighted");
                print;
                next;
            }
            {
                sub(/sdbpdl$/, "sdbpdl_weighted");
                print;
            }
        ' "$src_file" > "$dst_file"
        echo "$dst_file"
    else
        echo "$src_file"
    fi
}

merge_files_to_output() {
    local output_csv="$1"
    local label="$2"
    local -n in_files_ref="$3"

    if [[ ${#in_files_ref[@]} -eq 0 ]]; then
        return 1
    fi

    local merged_file="$TMP_DIR/merged_${label}.csv"
    local next_file="$TMP_DIR/next_${label}.csv"

    local first_file_prepared
    first_file_prepared="$(prepare_input_file "${in_files_ref[0]}")"
    cp "$first_file_prepared" "$merged_file"

    local expected_lines
    expected_lines="$(wc -l < "$first_file_prepared")"

    local merged_count=1
    local skipped_count=0

    local file
    for file in "${in_files_ref[@]:1}"; do
        local prepared_file
        prepared_file="$(prepare_input_file "$file")"
        local lines
        lines="$(wc -l < "$prepared_file")"
        if [[ "$lines" -ne "$expected_lines" ]]; then
            echo "Warning [$label]: skipping $file (line count $lines != expected $expected_lines)"
            skipped_count=$((skipped_count + 1))
            continue
        fi

        awk '
            NR == FNR {
                base[FNR] = $0;
                next;
            }
            {
                left = base[FNR];
                right = $0;

                commaPos = index(right, ",");
                tail = "";
                if (commaPos > 0) {
                    tail = substr(right, commaPos + 1);
                }

                if (left == "" || tail == "") {
                    print left;
                } else {
                    lastChar = substr(left, length(left), 1);
                    if (lastChar == ",") {
                        print left tail;
                    } else {
                        print left "," tail;
                    }
                }
            }
        ' "$merged_file" "$prepared_file" > "$next_file"

        mv "$next_file" "$merged_file"
        merged_count=$((merged_count + 1))
    done

    cp "$merged_file" "$output_csv"
    MERGE_RESULT_MERGED_COUNT="$merged_count"
    MERGE_RESULT_SKIPPED_COUNT="$skipped_count"
    return 0
}

DEFAULT_RUN_TYPES=("sd" "bp" "dl" "sdbpdl" "sdbpdl_weighted")
SELECTED_RUN_TYPES=()
if [[ "$RUN_TYPES_ARG" == "all" || -z "$RUN_TYPES_ARG" ]]; then
    SELECTED_RUN_TYPES=("${DEFAULT_RUN_TYPES[@]}")
else
    CLEANED_RUN_TYPES="${RUN_TYPES_ARG// /}"
    IFS=',' read -r -a REQUESTED_RUN_TYPES <<< "$CLEANED_RUN_TYPES"
    for run_type in "${REQUESTED_RUN_TYPES[@]}"; do
        case "$run_type" in
            sd|bp|dl|sdbpdl|sdbpdl_weighted)
                SELECTED_RUN_TYPES+=("$run_type")
                ;;
            *)
                echo "Error: unknown run type '$run_type'. Allowed: sd,bp,dl,sdbpdl,sdbpdl_weighted"
                exit 1
                ;;
        esac
    done
fi

merge_files_to_output "$OUTPUT_CSV" "all" FILES
MERGED_COUNT_ALL="$MERGE_RESULT_MERGED_COUNT"
SKIPPED_COUNT_ALL="$MERGE_RESULT_SKIPPED_COUNT"

echo "Merged $MERGED_COUNT_ALL files into: $OUTPUT_CSV"

TOTAL_TYPED_FILES_CREATED=0
for run_type in "${SELECTED_RUN_TYPES[@]}"; do
    mapfile -t TYPE_FILES < <(awk -F'|' -v t="$run_type" '$4 == t {print $5}' "$SORTED_ROWS_FILE")
    if [[ ${#TYPE_FILES[@]} -eq 0 ]]; then
        echo "No files found for run type: $run_type"
        continue
    fi

    TYPE_OUTPUT_CSV="$(build_typed_output_path "$OUTPUT_CSV" "$run_type")"
    merge_files_to_output "$TYPE_OUTPUT_CSV" "$run_type" TYPE_FILES
    echo "Merged $MERGE_RESULT_MERGED_COUNT $run_type files into: $TYPE_OUTPUT_CSV"
    TOTAL_TYPED_FILES_CREATED=$((TOTAL_TYPED_FILES_CREATED + 1))
done

echo "Total discovered files: ${#FILES[@]}"
echo "Search root: $RESULTS_ROOT"
echo "Search pattern: $INPUT_PATTERN"
if [[ "$RUN_TYPES_ARG" == "all" || -z "$RUN_TYPES_ARG" ]]; then
    echo "Run types requested: all"
else
    echo "Run types requested: $RUN_TYPES_ARG"
fi
if [[ $SKIPPED_COUNT_ALL -gt 0 ]]; then
    echo "Some files were skipped in all-run merge due to line-count mismatch."
fi
echo "Per-run-type output files created: $TOTAL_TYPED_FILES_CREATED"