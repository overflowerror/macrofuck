#!/bin/sh

tmpfile1="/tmp/$$-1.tmp"
tmpfile2="/tmp/$$-2.tmp"

ls cases/*.in test/cases/*.in | sed -E 's/.in$//g' |  while read test; do
  brainfuck < "$test.in" > $tmpfile1 2> $tmpfile2
  if test ! -e "$test.stdout"; then
    cp "$tmpfile1" "$test.stdout"
  fi

  if test ! -e "$test.stderr"; then
    cp "$tmpfile2" "$test.stderr"
  fi
done

rm "$tmpfile1"
rm "$tmpfile2"