#/bin/sh

executable="$1"

tmpfile1="/tmp/$$-1.tmp"
tmpfile2="/tmp/$$-2.tmp"
resultsfile="/tmp/$$.results"

touch "$resultsfile"

run_testcase() {
	test="$1"
	echo "Test: $test"

	if "$executable" "$test.in" < "$test.stdin" > "$tmpfile1" 2> "$tmpfile2" ; then
		if diff -q "$test.stderr" "$tmpfile2" > /dev/null; then
		  if diff -q "$test.stdout" "$tmpfile1" > /dev/null; then
        printf "  \033[32msuccess\033[0m\n"
        echo "$test: success" >> "$resultsfile"
		  else
        printf "  \033[31mfailed with stdout diff\033[0m\n"
        echo "$test: fail" >> "$resultsfile"
		  fi
		else
			printf "  \033[31mfailed with stderr diff\033[0m\n"
			echo "$test: fail" >> "$resultsfile"
		fi
	else
		printf "  \033[31mfailed with error\033[0m\n"
		echo "$test: fail" >> "$resultsfile"
	fi

	rm "$tmpfile1";
	rm "$tmpfile2";
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
