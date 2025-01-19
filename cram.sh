#!/usr/bin/env bash

set -euo pipefail

usage() {
    echo "Usage: $0 <command> [test-file]"
    echo "Commands:"
    echo "  diff   - Show colored word-level diff for .t files"
    echo "  accept - Accept new output for .t files"
    echo ""
    echo "If no test-file is specified, operates on all .t files in current directory"
    exit 1
}

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    usage
fi

command="$1"

# If no file specified, get all .t files in current directory
if [ $# -eq 2 ]; then
    test_files=("$2")
else
    # First check if any .t files exist to avoid literal "*.t" expansion
    shopt -s nullglob
    test_files=(*.t)
    shopt -u nullglob
    if [ ${#test_files[@]} -eq 0 ]; then
        echo "Error: No .t files found in current directory"
        exit 1
    fi
fi

process_file() {
    local test_file="$1"
    local ret=0

    # Verify the test file exists and ends in .t
    if [ ! -f "$test_file" ]; then
        echo "Error: Test file '$test_file' not found" >&2
        return 1
    fi

    if [[ ! "$test_file" =~ \.t$ ]]; then
        echo "Error: Test file must end in .t" >&2
        return 1
    fi

    # Get the path to the correction file
    local correction_file="${test_file}.err"

    case "$command" in
        "diff")
            if [ -f "$correction_file" ]; then
                git diff --no-index --word-diff --color-words --unified=3 \
                    "$test_file" "$correction_file" || ret=$?
            fi
            ;;
        "accept")
            if [ -f "$correction_file" ]; then
                cp "$correction_file" "$test_file"
                rm "$correction_file"
                echo "Accepted new output for $test_file"
            fi
            ;;
    esac

    return "$ret"
}

case "$command" in
    "diff"|"accept")
        exit_status=0
        for test_file in "${test_files[@]}"; do
            if ! process_file "$test_file"; then
                exit_status=1
            fi
        done
        exit "$exit_status"
        ;;
    *)
        echo "Error: Unknown command '$command'" >&2
        usage
        ;;
esac
