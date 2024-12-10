#!/bin/sh
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

obj="$1"
pages=$($SCRIPT_DIR/htelem-page-list.sh "$obj")
for page in $pages; do
    html_name=$(echo "$page" | cut -c4-)
    echo "writing ${html_name}"
    objcopy --dump-section "${page}=$html_name" "$obj"
done

