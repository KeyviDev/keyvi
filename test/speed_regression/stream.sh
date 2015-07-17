#!/bin/bash
d=$(pwd)
rm -rf stream.dat
for rev in 1644 1700 1800 HEAD; do
    cd $d
    if cd ../../tpie/ && svn up -r "$rev" && make clean && make && cd $d && make clean && make stream CFLAGS="-DREV=$rev"; then
		cd ../../tpie/
		R=$(svn info | sed -nre 's/Revision: //p')
		cd $d
		printf "$R" | tee -a stream.dat
		rm -rf tmp
		./stream | tee -a stream.dat
    else
		cd $d
		echo "$rev Build failed" | tee -a stream.dat
    fi
done
gnuplot stream.gp
if which okular >/dev/null; then
    okular stream.ps
elif which evince >/dev/null; then
    evince stream.ps
elif which gv >/dev/null; then
    gv stream.ps
fi