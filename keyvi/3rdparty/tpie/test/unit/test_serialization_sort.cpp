// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2013, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include "common.h"
#include <tpie/parallel_sort.h>
#include <tpie/serialization_sorter.h>
#include <tpie/sysinfo.h>
#include <boost/random.hpp>

using namespace tpie;

#include "merge_sort.h"

class use_serialization_sorter {
public:
	typedef std::vector<int> test_t;
	typedef serialization_sorter<test_t, std::less<test_t> > sorter;

	static void merge_runs(sorter & s) {
		s.merge_runs();
	}

	class item_generator {
	private:
		stream_size_type m_items;
		boost::rand48 m_rng;
		const static size_t expectedSize = 24;
		const static size_t modulus = 2*expectedSize;
		stream_size_type m_generated;

	public:
		item_generator(stream_size_type bytes)
			: m_items(bytes / sizeof(int) / expectedSize)
			, m_generated(0)
		{
		}

		~item_generator() {
			log_info() << "serialization_sort item generator generated " << m_generated*sizeof(int) << " bytes" << std::endl;
		}

		stream_size_type items() const { return m_items; }

		test_t operator()() {
			size_t length = m_rng() % modulus;
			test_t res(length);
			m_generated += length;
			std::generate(res.begin(), res.end(), m_rng);
			return res;
		}
	};
};

int main(int argc, char ** argv) {
	tests t(argc, argv);
	return
		sort_tester<use_serialization_sorter>::add_all(t)
		;
}
