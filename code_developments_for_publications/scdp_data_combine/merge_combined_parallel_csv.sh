#!/usr/bin/env bash
set -euo pipefail

# Merge all combined_data_parallel.csv files produced by parallel runs into
# one combined_data.csv style file (column-wise, like sequential mode).
#
# Usage:
#   ./merge_combined_parallel_csv.sh <results_root_dir> [output_csv] [input_pattern]
#
# Example:
#   ./merge_combined_parallel_csv.sh \
#     /home/tessa/BSO-toolbox-V2/code_developments_for_publications/Data/Results_modify_non_delete_fragments
#
# Notes:
# - The script searches recursively in all subdirectories of results_root_dir.
# - input_pattern defaults to combined_data_parallel.csv.

RESULTS_ROOT="${1:-}"
OUTPUT_CSV="${2:-}"
INPUT_PATTERN="${3:-combined_data_parallel.csv}"

if [[ -z "$RESULTS_ROOT" ]]; then
    echo "Usage: $0 <results_root_dir> [output_csv] [input_pattern]"
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

for FILE in "${FOUND_FILES[@]}"; do
    RUN_DIR_BASENAME="$(basename "$(dirname "$FILE")")"

    DESIGN_ID="999999"
    if [[ "$RUN_DIR_BASENAME" =~ ^design_([0-9]+)_ ]]; then
        DESIGN_ID="${BASH_REMATCH[1]}"
    fi

    RUN_RANK="99"
    if [[ "$RUN_DIR_BASENAME" == *_sd_-n=* ]]; then
        RUN_RANK="1"
    elif [[ "$RUN_DIR_BASENAME" == *_bp_-n=* ]]; then
        RUN_RANK="2"
    elif [[ "$RUN_DIR_BASENAME" == *_dl_-n=* ]]; then
        RUN_RANK="3"
    elif [[ "$RUN_DIR_BASENAME" == *_sdbpdl_-n=* ]]; then
        RUN_RANK="4" # sdbpdl (non-weighted)
    elif [[ "$RUN_DIR_BASENAME" == *_sdbpdl_*_-n=* ]]; then
        RUN_RANK="5" # sdbpdl_weighted
    fi

    # Keep deterministic order for duplicates by using full directory name.
    printf "%09d|%02d|%s|%s\n" "$DESIGN_ID" "$RUN_RANK" "$RUN_DIR_BASENAME" "$FILE" >> "$SORTED_LIST_FILE"
done

mapfile -t FILES < <(sort -t '|' -k1,1n -k2,2n -k3,3 "$SORTED_LIST_FILE" | cut -d'|' -f4-)

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

FIRST_FILE_PREPARED="$(prepare_input_file "${FILES[0]}")"
cp "$FIRST_FILE_PREPARED" "$TMP_DIR/merged.csv"
EXPECTED_LINES="$(wc -l < "$FIRST_FILE_PREPARED")"
MERGED_COUNT=1

for FILE in "${FILES[@]:1}"; do
    PREPARED_FILE="$(prepare_input_file "$FILE")"
    LINES="$(wc -l < "$PREPARED_FILE")"
    if [[ "$LINES" -ne "$EXPECTED_LINES" ]]; then
        echo "Warning: skipping $FILE (line count $LINES != expected $EXPECTED_LINES)"
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
    ' "$TMP_DIR/merged.csv" "$PREPARED_FILE" > "$TMP_DIR/next.csv"

    mv "$TMP_DIR/next.csv" "$TMP_DIR/merged.csv"
    MERGED_COUNT=$((MERGED_COUNT + 1))
done

cp "$TMP_DIR/merged.csv" "$OUTPUT_CSV"

echo "Merged $MERGED_COUNT files into: $OUTPUT_CSV"
echo "Total discovered files: ${#FILES[@]}"
echo "Search root: $RESULTS_ROOT"
echo "Search pattern: $INPUT_PATTERN"
if [[ $MERGED_COUNT -lt ${#FILES[@]} ]]; then
    echo "Some files were skipped due to line-count mismatch."
fi