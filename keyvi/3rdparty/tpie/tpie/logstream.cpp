// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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
#include <vector>
#include <algorithm>
#include <tpie/logstream.h>
#include <cstdio>
#include <tpie/tpie_log.h>
namespace tpie {

namespace log_bits {

std::vector<log_target *> log_targets;
bool logging_disabled;

}

using namespace log_bits;

log_stream_buf::log_stream_buf(log_level level) : m_level(level) {
	setp(m_buff, m_buff+buff_size-2);
}

log_stream_buf::~log_stream_buf() {flush();}

void log_stream_buf::flush() {
	if (pptr() == m_buff) return;
	if (!logging_disabled) {
		*pptr() = 0;
		if (log_targets.empty())
			// As a special service if no one is listening
			fwrite(m_buff, 1, pptr() - m_buff, stderr);
		else
			for(size_t i = 0; i < log_targets.size(); ++i)
				log_targets[i]->log(m_level, m_buff, pptr() - m_buff);
	}
	setp(m_buff, m_buff+buff_size-2);
}

int log_stream_buf::overflow(int c) {
	flush();
	*pptr() = static_cast<char>(c);
	pbump(1);
	return c;
}

int log_stream_buf::sync() {
	//Do not display the messages before there is a target
	if (log_targets.empty()) return 0;
	flush();
	return 0;
}

void add_log_target(log_target * t) {
	log_targets.push_back(t);
}

void remove_log_target(log_target * t) {
	std::vector<log_target *>::iterator i =
		std::find(log_targets.begin(), log_targets.end(), t);
	if (i != log_targets.end()) {
		log_bits::flush_logs();
		log_targets.erase(i);
	}
}

void begin_log_group(const std::string & name) {
	for(size_t i = 0; i < log_targets.size(); ++i)
		log_targets[i]->begin_group(name);
}

void end_log_group() {
	for(size_t i = 0; i < log_targets.size(); ++i)
		log_targets[i]->end_group();
}

log_group::log_group(const std::string & name)  {
	begin_log_group(name);
}

log_group::~log_group()  {
	end_log_group();
}

} //namespace tpie


