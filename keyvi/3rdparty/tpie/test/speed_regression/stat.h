// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

namespace tpie {
namespace test {

class stat {
	static const int width = 16;

public:
	inline stat(const std::vector<const char *> & name) {
		mean.resize(name.size(), 0);
		m2.resize(name.size(), 0);
		n = 0;
		next_col = 0;
		std::cout << std::setw(width) << "Test";
		for(size_t i=0; i < name.size(); ++i) 
			std::cout << std::setw(width) << name[i];
		std::cout << std::endl;
	}

	template <typename T>
	inline void operator()(const T & time) {
		if (next_col == 0) {
			++n;
			output(n);
		}
		output(time);

		double ftime = static_cast<double>(time);
		double delta = ftime - mean[next_col];
		mean[next_col] += static_cast<double>(delta) / static_cast<double>(n);
		m2[next_col] = m2[next_col] + delta*(ftime - mean[next_col]);

		++next_col;
		if (next_col == mean.size()) {
			next_col = 0;
			std::cout << '\n';
			print_mean_line();
			std::cout << '\r' << std::flush;
		}
	}

	inline ~stat() {
		print_mean_line();
		std::cout << std::endl << std::setw(width) << "stddev";
		for(size_t i=0; i < mean.size(); ++i) 
			output(sqrt(m2[i]/static_cast<double>(n-1)));
		std::cout << std::endl;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Output means
	///////////////////////////////////////////////////////////////////////////
	void print_mean_line() {
		std::cout << std::setw(width) << "mean";
		for(size_t i=0; i < mean.size(); ++i) 
			output(mean[i]);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief The number of digits to display after the decimal point to show
	/// the given amount of significant digits
	/// \tparam T Numeric type
	/// \tparam digits Number of significant digits to show
	/// \param time Target number
	/// \param base Internal
	///////////////////////////////////////////////////////////////////////////
	template <typename T, int digits>
	inline int precisionof(const T & time, T base = 1) {
		if (!digits) return 0;
		if (time < base) return digits;
		return precisionof<T, (digits > 0) ? digits-1 : 0>(time, base*10);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Output a number in a tabular cell, displaying 6 significant
	/// digits
	///////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline void output(const T & time) {
		std::cout << std::setw(width) << std::setprecision(precisionof<T, 6>(time)) << std::fixed << time << std::flush;
	}

	std::vector<double> mean;
	std::vector<double> m2;
	size_t n;
	size_t next_col;
};

} // namespace test
} // namespace tpie
