#/bin/sh

executable="$1"

tmpfile="/tmp/$$.tmp"
resultsfile="/tmp/$$.results"
touch "$resultsfile"

run_testcase() {
	test="$1"
	echo "Test: $test"

	if "$executable" -o "$tmpfile" "$test.in"; then
		if diff -q "$test.out" "$tmpfile" > /dev/null; then
			printf "  \033[32msuccess\033[0m\n"
			echo "$test: success" >> "$resultsfile"
		else
			printf "  \033[31mfailed with diff\033[0m\n"
			echo "$test: fail" >> "$resultsfile"
		fi
	else
		printf "  \033[31mfailed with error\033[0m\n"
		echo "$test: fail" >> "$resultsfile"
	fi

	rm "$tmpfile";
}

ls cases/*.in test/cases/*.in | sed -E 's/.in$//g' |  while read test; do
	run_testcase "$test"
done

echo
echo
echo "$(cat "$resultsfile" | wc -l) tests in total"
echo "$(grep ": fail" "$resultsfile" | wc -l) tests failed"
echo "$(grep ": success" "$resultsfile" | wc -l) tests succeeded" 
echo

rm "$resultsfile"
