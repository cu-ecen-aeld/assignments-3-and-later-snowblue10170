#!/bin/bash

if [ $# -lt 2 ]; then
	echo "error: parameters number wrong"
	exit 1
fi

writefile="$1"
writestr="$2"

mkdir -p "$(dirname "$writefile")"

echo "$writestr" > "$writefile"

if [ $? -ne 0 ]; then
	echo "error: can not create file $writefile。"
	exit 1
fi

exit 0

