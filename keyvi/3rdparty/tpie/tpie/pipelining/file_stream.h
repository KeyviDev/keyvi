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

namespace tpie {

namespace pipelining {

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

	inline input_t(dest_t dest, file_stream<item_type> & fs) : dest(std::move(dest)), fs(fs) {
		add_push_destination(this->dest);
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		if (fs.is_open()) {
			forward("items", fs.size());
		} else {
			forward("items", 0);
		}
		set_steps(fs.size());
	}

	virtual void go() override {
		if (fs.is_open()) {
			while (fs.can_read()) {
				dest.push(fs.read());
				step();
			}
		}
	}

private:
	dest_t dest;
	file_stream<item_type> & fs;
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

	inline pull_input_t(file_stream<T> & fs) : fs(fs) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		forward("items", fs.size());
		set_steps(fs.size());
	}

	inline T pull() {
		step();
		return fs.read();
	}

	inline bool can_pull() {
		return fs.can_read();
	}

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

	inline pull_reverse_input_t(file_stream<T> & fs) : fs(fs) {
		set_name("Read", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	virtual void propagate() override {
		forward("items", fs.size());
		set_steps(fs.size());
	}

	inline T pull() {
		step();
		return fs.read_back();
	}

	inline bool can_pull() {
		return fs.can_read_back();
	}

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

	inline output_t(file_stream<T> & fs) : fs(fs) {
		set_name("Write", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(fs.memory_usage());
	}

	inline void push(const T & item) {
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

	inline pull_output_t(source_t source, file_stream<item_type> & fs) : source(std::move(source)), fs(fs) {
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

	source_t source;
	file_stream<item_type> & fs;
};

template <typename T>
class tee_t {
public:
	template <typename dest_t>
	class type: public node {
	public:
		typedef T item_type;
		type(dest_t dest, file_stream<item_type> & fs): fs(fs), dest(std::move(dest)) {
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
};

template <typename T>
class pull_tee_t {
public:
	template <typename source_t>
	class type: public node {
	public:
		typedef T item_type;
		type(source_t source, file_stream<item_type> & fs): fs(fs), source(std::move(source)) {
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
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that pushes the contents of the given file stream
/// to the next node in the pipeline.
/// \param fs The file stream from which it pushes items
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pipe_begin<factory<bits::input_t, file_stream<T> &> > input(file_stream<T> & fs) {
	return factory<bits::input_t, file_stream<T> &>(fs);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items from the given file_stream
/// \param fs The file stream from which it reads items.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::pull_input_t<T>, file_stream<T> &> > pull_input(file_stream<T> & fs) {
	return termfactory<bits::pull_input_t<T>, file_stream<T> &>(fs);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining pull-node that reads items in reverse order from the
/// given file_stream
/// \param fs The file stream from which it reads items.
///////////////////////////////////////////////////////////////////////////////
template<typename T>
inline pullpipe_begin<termfactory<bits::pull_reverse_input_t<T>, file_stream<T> &> > pull_reverse_input(file_stream<T> & fs) {
	return termfactory<bits::pull_reverse_input_t<T>, file_stream<T> &>(fs);
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
	return termfactory<bits::output_t<T>, file_stream<T> &>(fs);
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
	return factory<bits::pull_output_t, file_stream<T> &>(fs);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes the pushed to a file stream and then
/// pushes the items to the next node.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_middle<factory<bits::tee_t<typename push_type<T>::type>::template type, T &> >
tee(T & fs) {return factory<bits::tee_t<typename push_type<T>::type>::template type, T &>(fs);}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that when pulled from will pull from its source,
/// write its item to disk and then return the item.
/// \param fs The file stream that items should be written to
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pullpipe_middle<factory<bits::pull_tee_t<typename push_type<T>::type>::template type, T &> >
pull_tee(T & fs) {return factory<bits::pull_tee_t<typename push_type<T>::type>::template type, T &>(fs);}

} // namespace pipelining

} // namespace tpie
#endif
