// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#include <tpie/pipelining/merge_sorter.h>

namespace tpie {

namespace bits {

run_positions::run_positions()
	: m_open(false)
{
}

run_positions::~run_positions() {
	close();
}

/*static*/ memory_size_type run_positions::memory_usage() {
	return sizeof(run_positions)
		+ 2 * file_stream<stream_position>::memory_usage();
}

void run_positions::open() {
	m_positions[0].open(m_positionsFile[0]);
	m_positions[1].open(m_positionsFile[1]);
	m_runs[0] = m_runs[1] = 0;
	m_levels = 1;
	m_open = true;
	m_final = m_evacuated = false;
	m_finalExtraSet = false;
	m_finalExtra = stream_position();
	m_finalPositions.resize(0);
}

void run_positions::close() {
	if (m_open) {
		m_positions[0].close();
		m_positions[1].close();
		m_open = m_final = m_evacuated = false;
		m_finalExtraSet = false;
		m_finalExtra = stream_position();
		m_finalPositions.resize(0);
	}
}

void run_positions::evacuate() {
	if (m_evacuated) return;
	m_evacuated = true;
	if (m_final) {
		log_debug() << "run_positions::evacuate while final" << std::endl;
		m_positionsFile[0].free();
		m_positions[0].open(m_positionsFile[0]);
		m_positions[0].write(m_finalPositions.begin(), m_finalPositions.end());
		m_finalPositions.resize(0);
		m_positions[0].close();
	} else {
		log_debug() << "run_positions::evacuate while not final" << std::endl;
		m_positionsPosition[0] = m_positions[0].get_position();
		m_positionsPosition[1] = m_positions[1].get_position();
		m_positions[0].close();
		m_positions[1].close();
	}
}

void run_positions::unevacuate() {
	if (!m_evacuated) return;
	m_evacuated = false;
	if (m_final) {
		log_debug() << "run_positions::unevacuate while final" << std::endl;
		m_positions[0].open(m_positionsFile[0]);
		m_finalPositions.resize(static_cast<memory_size_type>(m_positions[0].size()));
		m_positions[0].read(m_finalPositions.begin(), m_finalPositions.end());
		m_positions[0].close();
	} else {
		log_debug() << "run_positions::unevacuate while not final" << std::endl;
		m_positions[0].open(m_positionsFile[0]);
		m_positions[1].open(m_positionsFile[1]);
		m_positions[0].set_position(m_positionsPosition[0]);
		m_positions[1].set_position(m_positionsPosition[1]);
	}
}

void run_positions::next_level() {
	if (m_final)
		throw exception("next_level: m_final == true");
	if (m_evacuated)
		throw exception("next_level: m_evacuated == true");
	if (!m_open)
		throw exception("next_level: m_open == false");
	m_positions[m_levels % 2].truncate(0);
	++m_levels;
	m_positions[m_levels % 2].seek(0);

	m_runs[0] = m_runs[1] = 0;
}

void run_positions::final_level(memory_size_type fanout) {
	if (m_final)
		throw exception("final_level: m_final == true");
	if (m_evacuated)
		throw exception("final_level: m_evacuated == true");
	if (!m_open)
		throw exception("final_level: m_open == false");

	m_final = true;
	file_stream<stream_position> & s = m_positions[m_levels % 2];
	if (fanout > s.size() - s.offset()) {
		log_debug() << "Decrease final level fanout from " << fanout << " to ";
		fanout = static_cast<memory_size_type>(s.size() - s.offset());
		log_debug() << fanout << std::endl;
	}
	m_finalPositions.resize(fanout);
	s.read(m_finalPositions.begin(), m_finalPositions.end());
	m_positions[0].close();
	m_positions[1].close();
}

void run_positions::set_position(memory_size_type mergeLevel, memory_size_type runNumber, stream_position pos) {
	if (!m_open) open();

	if (mergeLevel+1 != m_levels) {
		throw exception("set_position: incorrect mergeLevel");
	}
	if (m_final) {
		log_debug() << "run_positions set_position setting m_finalExtra" << std::endl;
		m_finalExtra = pos;
		m_finalExtraSet = true;
		return;
	}
	file_stream<stream_position> & s = m_positions[mergeLevel % 2];
	memory_size_type & expectedRunNumber = m_runs[mergeLevel % 2];
	if (runNumber != expectedRunNumber) {
		throw exception("set_position: Wrong run number");
	}
	++expectedRunNumber;
	s.write(pos);
}

stream_position run_positions::get_position(memory_size_type mergeLevel, memory_size_type runNumber) {
	if (!m_open) throw exception("get_position: !open");

	if (m_final && mergeLevel+1 == m_levels) {
		log_debug() << "run_positions get_position returning m_finalExtra" << std::endl;
		if (!m_finalExtraSet)
			throw exception("get_position: m_finalExtra uninitialized");
		return m_finalExtra;
	}

	if (mergeLevel+2 != m_levels) {
		throw exception("get_position: incorrect mergeLevel");
	}
	if (m_final) {
		return m_finalPositions[runNumber];
	}
	file_stream<stream_position> & s = m_positions[mergeLevel % 2];
	memory_size_type & expectedRunNumber = m_runs[mergeLevel % 2];
	if (runNumber != expectedRunNumber) {
		throw exception("get_position: Wrong run number");
	}
	++expectedRunNumber;
	if (!s.can_read())
		throw exception("get_position: !can_read");
	return s.read();
}


} // namespace bits

} // namespace tpie
