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

set(Boost_USE_MULTITHREADED ON)
if(WIN32)
	set(Boost_USE_STATIC_LIBS    ON)
endif(WIN32)
find_package(Boost COMPONENTS date_time thread filesystem system regex)

find_path(TPIE_INCLUDE_DIR NAMES tpie/config.h)
find_library(TPIE_LIBRARIES tpie)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TPIE DEFAULT_MSG TPIE_LIBRARIES TPIE_INCLUDE_DIR)
list(APPEND TPIE_LIBRARIES ${Boost_LIBRARIES})
