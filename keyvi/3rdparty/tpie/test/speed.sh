#!/bin/sh

# Copyright 2011, The TPIE development team
# 
# This file is part of TPIE.
# 
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

# A batch tester for speed regression testing.
# If your builds are in tpie-foo/build, tpie-bar/build and tpie-baz/build (and
# your test binaries in .../build/test), run the script with:
#     sh speed.sh tpie-foo/build tpie-bar/build tpie-baz/build
# You can use the speedparse.pl script to parse the log output to CSV.

#SORTITEMS=16777216
#PQITEMS=8388608
#PQMEMORY=33554432
#TIMES=4
#STREAMCOUNT=32768
SORTITEMS=4294967296
PQITEMS=4294967296
PQMEMORY=33554432
TIMES=2

while true; do

  # Enter test command lines here.
  #for prog in 'speed_regression/stream_speed_test $TIMES $STREAMCOUNT' 'test_ami_sort -v -t $SORTITEMS' 'speed_regression/pq_speed_test $TIMES $PQITEMS $PQMEMORY'; do
  for prog in 'speed_regression/pq_speed_test $TIMES $PQITEMS $PQMEMORY' 'test_ami_sort -v -t $SORTITEMS'; do

    for dir in $@; do
      echo '==============================================================================='
      echo 'BEGIN TEST'
      eval "echo \"Program: $prog\""
      echo "Directory: $dir"
      echo
      mkdir testspeed
      cd testspeed
      eval "time $dir/test/$prog"
      cd ..
      rm -rf testspeed
      echo
      echo 'END TEST'
    done
  done

  # Uncomment the following lines to increase the values with each run.
  #SORTITEMS=`echo "2*$SORTITEMS"|bc`
  #PQITEMS=`echo "2*$PQITEMS"|bc`
  #STREAMCOUNT=`echo "2*$STREAMCOUNT"|bc`
done
