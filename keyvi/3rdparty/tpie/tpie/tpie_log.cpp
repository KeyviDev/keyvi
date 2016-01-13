// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011 The TPIE development team
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

// We are logging
#define TPL_LOGGING	1

#include <cstdlib>
#include <time.h>
#include <tpie/tempname.h>
#include <tpie/logstream.h>
#include <tpie/tpie_log.h>
#include <iostream>

tpie::file_log_target::file_log_target(log_level threshold): m_threshold(threshold) {
	m_path = tempname::tpie_name("log", "" , "txt");
	m_out.open(m_path.c_str(), std::ios::trunc | std::ios::out);
}

void tpie::file_log_target::log(log_level level, const char * message, size_t) {
	if (level > m_threshold) return;

	if(LOG_DEBUG > level) { // print without indentation
		m_out << message;
		m_out.flush();
		return;
	}

	m_out << build_prefix(groups.size()) << " " << message;
	m_out.flush();
}

std::string tpie::file_log_target::build_prefix(size_t size) {
	return std::string(size, '|');
}

void tpie::file_log_target::begin_group(const std::string & name) {
	if(LOG_DEBUG > m_threshold) return;

	groups.push(name);

	m_out << build_prefix(groups.size()-1) << "> " << "Entering " << name << std::endl;
}

void tpie::file_log_target::end_group() {
	if(LOG_DEBUG > m_threshold) return;

	m_out << build_prefix(groups.size()-1) << "x " << "Leaving " << groups.top() << std::endl;
	groups.pop();
}

tpie::stderr_log_target::stderr_log_target(log_level threshold): m_threshold(threshold) {}

std::string tpie::stderr_log_target::build_prefix(size_t size) {
	std::string prefix;
	for(size_t i = 0; i < size; ++i) prefix += "|";
	return prefix;
}

void tpie::stderr_log_target::log(log_level level, const char * message, size_t size) {
	if (level > m_threshold) return;

	if(LOG_DEBUG > level) { // print without indentation
		fwrite(message, 1, size, stderr);
		return;
	}

	std::string prefix = build_prefix(groups.size()) + " ";

	fwrite(prefix.c_str(), 1, prefix.size(), stderr);
	fwrite(message, 1, size, stderr);

}	

void tpie::stderr_log_target::begin_group(const std::string & name) {
	if(LOG_DEBUG > m_threshold) return;

	groups.push(name);
	
	std::string prefix = build_prefix(groups.size()-1) + "> ";
	std::string text = "Entering " + name + "\n";

	fwrite(prefix.c_str(), sizeof(char), prefix.size(), stderr);
	fwrite(text.c_str(), sizeof(char), text.size(), stderr);
}

void tpie::stderr_log_target::end_group() {
	if(LOG_DEBUG > m_threshold) return;

	std::string text = "Leaving " + groups.top() + "\n";
	std::string prefix = build_prefix(groups.size()-1) + "x ";

	groups.pop();

	fwrite(prefix.c_str(), sizeof(char), prefix.size(), stderr);	
	fwrite(text.c_str(), sizeof(char), text.size(), stderr);
}








namespace tpie {

static file_log_target * file_target = 0;
static stderr_log_target * stderr_target = 0;

namespace log_bits {

bool log_selector::s_init;
log_level log_selector::s_level;

std::vector<std::shared_ptr<logstream> > log_instances;

} // namespace log_bits

const std::string& log_name() {
	return file_target->m_path;
}

void init_default_log() {
	if (file_target) return;
	file_target = new file_log_target(LOG_DEBUG);
	stderr_target = new stderr_log_target(LOG_INFORMATIONAL);
	add_log_target(file_target);
	add_log_target(stderr_target);
}

void finish_default_log() {
	if (!file_target) return;
	remove_log_target(file_target);
	remove_log_target(stderr_target);
	delete file_target;
	delete stderr_target;
	file_target = 0;
	stderr_target = 0;
}

namespace log_bits {

void initiate_log_level(log_level level) {
	while (log_instances.size() <= level)
		log_instances.push_back(std::shared_ptr<logstream>());
	log_instances[level] = std::make_shared<logstream>(level);
}

void flush_logs() {
	for (size_t i = 0; i < log_instances.size(); ++i) {
		if (log_instances[i].get() != 0) {
			*log_instances[i] << std::flush;
		}
	}
}

} // namespace log_bits

} //namespace tpie
