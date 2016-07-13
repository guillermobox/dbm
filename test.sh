#!/bin/bash

function setup() {
	export DBM_PATH="/tmp/test.db"
	rm -f "$DBM_PATH"
}

function check() {
	setup
	output=$(eval "$2" 2>/dev/null)
	errcode=$?

	if [ "$errcode" != "$4" -o "$output" != "$3" ]; then
		echo "$output"
		printf "%20s \033[31m WRONG\033[0m\n" "$1"
	else
		printf "%20s \033[32m RIGHT\033[0m\n" "$1"
	fi
}

OIFS="$IFS"
cat tests.txt | while read line; do
	IFS="|"
	read name command output error <<< "$line"
	IFS="$OIFS"
	check "$name" "$command" "$output" "$error"
done
