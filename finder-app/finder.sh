#!/bin/bash

if [ $# -lt 2 ]; then
	echo "error: parameters number wrong"
	exit 1
fi

filesdir="$1"
searchstr="$2"

if [ ! -d "$filesdir" ]; then
	echo "error:$filesdir is not a directory"
	exit 1
fi

lines=0
while IFS= read -r -d '' file; do
	count=$(grep -c "$searchstr" "$file")
	lines=$((lines + count))
done < <(find "$filesdir" -type f -print0)

files=$(find "$filesdir" -type f |wc -l)

echo "The number of files are $files and the number of matching lines are $lines"

exit 0
