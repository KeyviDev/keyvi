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

PQITEMS=46976204800 # 350 GB uint64_t's
PQMEMORY=50331648

rm -rf testspeed

for bs in 0.0078125 0.015625 0.03125 0.0625 0.125 0.25 0.5 1.0; do
  echo '==============================================================================='
  echo 'BEGIN TEST'
  echo "Block size: $bs"
  echo
  mkdir testspeed
  cd testspeed
  /home/rav/work/tpie/Release/test/speed_regression/pq_speed_test -b $bs 1 $PQITEMS $PQMEMORY 2>&1
  cd ..
  rm -rf testspeed
  echo
  echo 'END TEST'
done
