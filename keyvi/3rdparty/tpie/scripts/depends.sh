#!/bin/sh
set -e
out=depends.pdf
driver=dot
if [ $# -gt 0 ]; then
	driver=$1
	shift
fi
(
	echo 'digraph {'
	echo 'rankdir=LR;'
	for i in *.h *.cpp *.inl; do
		echo "\"$i\";"
	done
	for i in *.h *.inl; do
		echo -n . >&2
		find -not -name "$i" \( -name "*.h" -or -name "*.inl" -or -name "*.cpp" \) -print0 \
		| xargs -0 grep -l "#include.*[</\"]$i" \
		| sed -e 's/^\.\///' \
		| while read line; do
			echo "\"$line\" -> \"$i\";"
		done
	done
	echo >&2
	echo '}'
) \
| egrep -v 'deadcode' > depends.dot
time $driver -Tpdf -o$out depends.dot && xdg-open $out
# vim:set ts=4 sw=4 sts=4 noet:
