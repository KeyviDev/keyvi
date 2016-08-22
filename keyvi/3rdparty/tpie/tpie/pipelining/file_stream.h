// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_FILE_STREAM_H__
#define __TPIE_PIPELINING_FILE_STREAM_H__

#include <tpie/file_stream.h>

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/maybe.h>
#include <tpie/flags.h>

namespace tpie {
namespace pipelining {

enum stream_option {
	STREAM_RESET=1,
	STREAM_CLOSE=2
};

TPIE_DECLARE_OPERATORS_FOR_FLAGS(stream_option)
typedef tpie::flags<stream_option> stream_options;

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \class input_t
///
/// file_stream input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class input_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	input_t(dest_t dest, file_stream<item_type> & fs, stream_options options) : options(options), fs(fs), dest(std::move(dest)) {
		add_push_destination(this->dest);
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		if (options & STREAM_RESET) fs.seek(0);
		
		if (fs.is_open()) {
			forward("items", fs.size() - fs.offset());
			set_steps(fs.size() - fs.offset());
		} else {
			forward("items", 0);
		}
	}

	virtual void go() override {
		if (fs.is_open()) {
			while (fs.can_read()) {
				dest.push(fs.read());
				step();
			}
		}
	}

	virtual void end() override {
		if (options & STREAM_CLOSE) fs.close();
	}

private:
	stream_options options;
	file_stream<item_type> & fs;
	dest_t dest;
};

///////////////////////////////////////////////////////////////////////////////
/// \class input_t
///
/// file_stream input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class named_input_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	named_input_t(dest_t dest, std::string path) : dest(std::move(dest)), path(path) {
		add_push_destination(this->dest);
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(file_stream<item_type>::memory_usage());
	}

	virtual void propagate() override {
		fs.construct();
		fs->open(path, access_read);
		forward("items", fs->size());
		set_steps(fs->size());
	}

	virtual void go() override {
		while (fs->can_read()) {
			dest.push(fs->read());
			step();
		}
		fs.destruct();
	}
private:
	dest_t dest;
	maybe<file_stream<item_type> > fs;
	std::string path;
};


///////////////////////////////////////////////////////////////////////////////
/// \class pull_input_t
///
/// file_stream pull input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class pull_input_t : public node {
public:
	typedef T item_type;

	pull_input_t(file_stream<T> & fs, stream_options options) : options(options), fs(fs) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		if (options & STREAM_RESET) fs.seek(0);
		forward("items", fs.size()-fs.offset());
		set_steps(fs.size()-fs.offset());
	}

	T pull() {
		step();
		return fs.read();
	}

	bool can_pull() {
		return fs.can_read();
	}

	virtual void end() override {
		if (options & STREAM_CLOSE) fs.close();
	}

private:
	stream_options options;
	file_stream<T> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pull_reverse_input_t
///
/// file_stream pull input generator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class pull_reverse_input_t : public node {
public:
	typedef T item_type;

	pull_reverse_input_t(file_stream<T> & fs, stream_options options) : options(options), fs(fs) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		if (options & STREAM_RESET) fs.seek(0, file_stream<T>::end);
		forward("items", fs.offset());
		set_steps(fs.offset());
	}

	inline T pull() {
		step();
		return fs.read_back();
	}

	inline bool can_pull() {
		return fs.can_read_back();
	}

	virtual void end() override {
		if (options & STREAM_CLOSE) fs.close();
	}

private:
	stream_options options;
	file_stream<T> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class named_output_t
///
/// file_stream output terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class named_pull_input_t : public node {
public:
	typedef T item_type;

	named_pull_input_t(std::string path): path(std::move(path)) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(file_stream<T>::memory_usage());
	}

	virtual void propagate() override {
		fs.construct();
		fs->open(path, access_read);
		forward("items", fs->size());
		set_steps(fs->size());
	}

	T pull() {
		step();
		return fs->read();
	}

	bool can_pull() {
		return fs->can_read();
	}

	void end() override {
		fs->close();
		fs.destruct();
	}
private:
	maybe<file_stream<T> > fs;
	std::string path;
};


///////////////////////////////////////////////////////////////////////////////
/// \class output_t
///
/// file_stream output terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class output_t : public node {
public:
	typedef T item_type;

	output_t(file_stream<T> & fs) : fs(fs) {
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	void push(const T & item) {
		fs.write(item);
	}
private:
	file_stream<T> & fs;
};

///////////////////////////////////////////////////////////////////////////////
/// \class named_output_t
///
/// file_stream output terminator.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class named_output_t : public node {
public:
	typedef T item_type;

	named_output_t(const std::string & path): path(path) {
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(file_stream<T>::memory_usage());
	}

	void begin() override {
		fs.construct();
		fs->open(path, access_write);
	}
	
	void push(const T & item) {
		fs->write(item);
	}

	void end() override {
		fs->close();
		fs.destruct();
	}
private:
	maybe<file_stream<T> > fs;
	std::string path;
};


///////////////////////////////////////////////////////////////////////////////
/// \class pull_output_t
///
/// file_stream output pull data source.
///////////////////////////////////////////////////////////////////////////////
template <typename source_t>
class pull_output_t : public node {
public:
	typedef typename pull_type<source_t>::type item_type;

	pull_output_t(source_t source, file_stream<item_type> & fs) : source(std::move(source)), fs(fs) {
		add_pull_source(this->source);
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void go() override {
		source.begin();
		while (source.can_pull()) {
			fs.write(source.pull());
		}
		source.end();
	}
	
private:
	source_t source;
	file_stream<item_type> & fs;
};

template <typename dest_t, typename T>
class tee_t: public node {
public:
	typedef T item_type;
	tee_t(dest_t dest, file_stream<item_type> & fs): fs(fs), dest(std::move(dest)) {
		set_minimum_memory(fs.memory_usage());
	}
	
	void push(const item_type & i) {
		fs.write(i);
		dest.push(i);
	}
private:
	file_stream<item_type> & fs;
	dest_t dest;
};

template <typename source_t, typename T>
class pull_tee_t {
public:
	typedef T item_type;
	pull_tee_t(source_t source, file_stream<item_type> & fs): fs(fs), source(std::move(source)) {
		set_minimum_memory(fs.memory_usage());
	}
		
	bool can_pull() {
		return source.can_pull();
	}
	
	item_type pull() {
		item_type i = source.pull();
		fs.write(i);
		return i;
	}
private:
	file_stream<item_type> & fs;
	source_t source;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given file stream
/// to the next node in the pipeline.
/// \param fs The file stream from which it pushes items
/// \param options Stream options
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pipe_begin<factory<bits::input_t, file_stream<T> &, stream_options> > input(file_stream<T> & fs,
																				   stream_options options=stream_options()) {
	return {fs, options};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the named file stream
/// to the next node in the pipeline.
/// \param path The file stream from which it pushes items
///////////////////////////////////////////////////////////////////////////////
typedef pipe_begin<factory<bits::named_input_t, std::string> > named_input; 

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items from the given file_stream
/// \param fs The file stream from which it reads items.
/// \param options Stream options
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::pull_input_t<T>, file_stream<T> &, stream_options> > pull_input(
	file_stream<T> & fs,
	stream_options options=stream_options()) {
	return {fs, options};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items in reverse order from the
/// given file_stream
/// \param fs The file stream from which it reads items.
/// \param options Stream options
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::pull_reverse_input_t<T>, file_stream<T> &, stream_options> > pull_reverse_input(
	file_stream<T> & fs,
	stream_options options=stream_options()) {
	return {fs, options};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items from the given file_stream
/// \param fs The file stream from which it reads items.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::named_pull_input_t<T>, std::string> > named_pull_input(std::string name) {
	return {std::move(name)};
}


///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes the pushed items to a file stream.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory<bits::output_t<T>, file_stream<T> &> > output(file_stream<T> & fs) {
	return {fs};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes the pushed items to a named file stream.
/// \param path The path of where to write the firestream
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory<bits::named_output_t<T>, std::string> > named_output(std::string path) {
	return {std::move(path)}; 
}


///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that writes the pulled items to a file stream.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_end<factory<bits::pull_output_t, file_stream<T> &> > pull_output(file_stream<T> & fs) {
	return {fs};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes the pushed to a file stream and then
/// pushes the items to the next node.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_middle<tfactory<bits::tee_t, Args<typename T::item_type>, T &> > tee(T & fs) {
	return {fs};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that when pulled from will pull from its source,
/// write its item to disk and then return the item.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pullpipe_middle<tfactory<bits::pull_tee_t, Args<typename T::item_type>, T &> > pull_tee(T & fs) {
	return {fs};
}

} // namespace pipelining

} // namespace tpie
#endif
