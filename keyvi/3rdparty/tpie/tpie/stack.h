// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2008, 2011, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file stack.h  External memory stack
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_AMI_STACK_H
#define _TPIE_AMI_STACK_H

#include <tpie/array.h>
#include <tpie/portability.h>
#include <tpie/deprecated.h>
#include <tpie/stream.h>
#include <tpie/compressed/stream.h>
#include <tpie/tpie_assert.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
/// \brief  An implementation of an external-memory stack.
///////////////////////////////////////////////////////////////////
template <typename T> 
class stack {
public:

    ////////////////////////////////////////////////////////////////////
    /// \brief Initialize anonymous stack.
    ////////////////////////////////////////////////////////////////////
	stack(double blockFactor=1.0)
		: m_stream(blockFactor)
		, m_buffer(buffer_size(blockFactor))
		, m_bufferItems(0)
	{
		m_stream.open(static_cast<memory_size_type>(0), access_normal,
					  compression_normal);
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Initialize named, nontemporary stack.
    ///
    /// \param  path    The path to a file used for storing the items.
    /// \param  blockFactor  The block factor to use
    ////////////////////////////////////////////////////////////////////
	stack(const std::string & path, double blockFactor=1.0)
		: m_stream(blockFactor)
		, m_buffer(buffer_size(blockFactor))
		, m_bufferItems(0)
	{
		m_stream.open(path, access_read_write,
					  static_cast<memory_size_type>(0), access_normal,
					  compression_normal);
		m_stream.seek(0, file_stream_base::end);
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Initialize temporary stack.
    ///
    /// \param  tempFile  The temporary file containing the stack
    /// \param  blockFactor  The block factor to use
    ////////////////////////////////////////////////////////////////////
	stack(temp_file & tempFile, double blockFactor=1.0)
		: m_stream(blockFactor)
		, m_buffer(buffer_size(blockFactor))
		, m_bufferItems(0)
	{
		m_stream.open(tempFile, access_read_write,
					  static_cast<memory_size_type>(0), access_normal,
					  compression_normal);
		m_stream.seek(0, file_stream_base::end);
	}


    ////////////////////////////////////////////////////////////////////
    /// \brief Closes the underlying stream and truncates it to the logical
    /// end of the stack.
    ////////////////////////////////////////////////////////////////////
	~stack() {
		empty_buffer();
		m_stream.truncate(m_stream.get_position());
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Pushes one item onto the stack. Returns ERROR_* as 
    /// given by the underlying stream.
    ///
    /// \param  t    The item to push onto the stack.
    ////////////////////////////////////////////////////////////////////
	inline void push(const T & t) throw(stream_exception) {
		if (m_buffer.size() == m_bufferItems) empty_buffer();
		m_buffer[m_bufferItems++] = t;
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Pops one item from the stack.
    ////////////////////////////////////////////////////////////////////
	inline const T & pop() throw(stream_exception) {
		if (m_bufferItems) return m_buffer[--m_bufferItems];
		const T & item = m_stream.read_back();
		return item;
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Peeks at the topmost item on the stack.
    ////////////////////////////////////////////////////////////////////
	inline const T & top() throw(stream_exception) {
		if (m_bufferItems) return m_buffer[m_bufferItems-1];
		m_buffer[0] = m_stream.read_back();
		m_stream.read();
		return m_buffer[0];
	}

    ////////////////////////////////////////////////////////////////////
    /// \brief Returns the number of items currently on the stack.
    ////////////////////////////////////////////////////////////////////
    stream_size_type size() const {
		return m_stream.offset()+m_bufferItems;
    }

    ////////////////////////////////////////////////////////////////////
    /// \brief Returns whether the stack is empty or not.
    ////////////////////////////////////////////////////////////////////
    inline bool empty() const {
		return size()  == 0;//!m_file_stream.can_read_back();
    }

    ////////////////////////////////////////////////////////////////////
    /// \brief Compute the memory used by a stack.
    ////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_usage(float blockFactor=1.0) {
		return sizeof(stack<T>)
			+ file_stream<T>::memory_usage(blockFactor)
			+ array<T>::memory_usage(buffer_size(blockFactor));
	}

protected:

	/** The stream used to store the items. */
	file_stream<T> m_stream;

private:
	array<T> m_buffer;
	size_t m_bufferItems;

	inline void empty_buffer() {
		if (m_bufferItems == 0) return;
		m_stream.truncate(m_stream.get_position());
		m_stream.write(m_buffer.begin(), m_buffer.begin()+m_bufferItems);
		m_bufferItems = 0;
	}

	static memory_size_type buffer_size(double blockFactor) {
		return file_stream<T>::block_size(blockFactor)/sizeof(T);
	}

};

namespace ami {

///////////////////////////////////////////////////////////////////
///  An implementation of an external-memory stack compatible with the old AMI interface.
///////////////////////////////////////////////////////////////////
template<class T> 
class stack {
public:
   
    ////////////////////////////////////////////////////////////////////
    ///  Initializes the stack.
    ////////////////////////////////////////////////////////////////////
    stack() :
		m_ulate(m_tempFile) {
		// Empty ctor.
	}

    ////////////////////////////////////////////////////////////////////
    ///  Initializes the stack by (re-)opening the file given.
    ///
    ///  \param  path    The path to a file used for storing the items.
    ///  \param  type    An stream_type that indicates the 
    ///                  read/write mode of the file.
    ////////////////////////////////////////////////////////////////////
    stack(const std::string& path, 
		  stream_type type = READ_WRITE_STREAM)
		: m_tempFile(path, true)
		, m_ulate(m_tempFile)
	{
		unused(type);
	}

    ////////////////////////////////////////////////////////////////////
    ///  Pushes one item onto the stack. Returns ERROR_* as 
    ///  given by the underlying stream.
    ///
    ///  \param  t    The item to be pushed onto the stack.
    ////////////////////////////////////////////////////////////////////
    err push(const T &t);

    ////////////////////////////////////////////////////////////////////
    ///  Pops one item from the stack. Returns ERROR_* as 
    ///  given by the underlying stream or END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ////////////////////////////////////////////////////////////////////
    err pop(const T **t); 

    ////////////////////////////////////////////////////////////////////
    ///  Peeks at the topmost item on the stack. Returns ERROR_* as 
    ///  given by the underlying stream or END_OF_STREAM
    ///  if the stack is empty.
    ///
    ///  \param  t    A pointer to a pointer that will point to the 
    ///               topmost item.
    ////////////////////////////////////////////////////////////////////
    err peek(const T **t); 

    ////////////////////////////////////////////////////////////////////
    ///  Returns the number of items currently on the stack.
    ////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET size() const {
		return m_ulate.size();
    }

    ////////////////////////////////////////////////////////////////////
    ///  Returns whether the stack is empty or not.
    ////////////////////////////////////////////////////////////////////
    bool is_empty() const {
		return m_ulate.empty();
    }

    ////////////////////////////////////////////////////////////////////
    ///  Set the persistence status of the (stream underlying the) stack.
    ///
    ///  \param  p    A persistence status.
    ////////////////////////////////////////////////////////////////////
	void persist(persistence p) {
		m_tempFile.set_persistent(p == PERSIST_PERSISTENT);
    }

    ////////////////////////////////////////////////////////////////////
    ///  Returns the persistence status of the (stream underlying the) 
    ///  stack.
    ////////////////////////////////////////////////////////////////////
	persistence persist() const { 
		return m_tempFile.is_persistent() ? PERSIST_PERSISTENT : PERSIST_DELETE;
    }

    ////////////////////////////////////////////////////////////////////
    ///  Truncates the underlying stream to the exact size (rounded up
    ///  to the next block) of items. In the current implementation,
	///  this does nothing.
    ////////////////////////////////////////////////////////////////////
    err trim() {
		return NO_ERROR;
    }

    ////////////////////////////////////////////////////////////////////
    ///  Compute the memory used by the stack and the aggregated stream.
    ///
    ///  \param  usage       Where the usage will be stored.
    ///  \param  usage_type  The type of usage_type inquired from 
    ///                      the stream.
    ////////////////////////////////////////////////////////////////////
	err main_memory_usage(TPIE_OS_SIZE_T *usage,
						  stream_usage usage_type) const;

    ////////////////////////////////////////////////////////////////////
    /// \deprecated This should go as soon as all old code has been migrated.
    ////////////////////////////////////////////////////////////////////
    TPIE_OS_OFFSET stream_len() const {
		TP_LOG_WARNING_ID("Using AMI_stack<T>::stream_len() is deprecated.");
		return size();
    }

	tpie::stack<T> & underlying_stack() {
		return m_ulate;
	}
private:

	temp_file m_tempFile;

	// em-ulate. get it?
	tpie::stack<T> m_ulate;
};

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::push(const T &t) {

    err retval = NO_ERROR;

	try {
		m_ulate.push(t);
	} catch (end_of_stream_exception &) {
		retval = END_OF_STREAM;
	} catch (stream_exception &) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::pop(const T **t) {
    if(m_ulate.empty())
        return END_OF_STREAM;

    err retval = NO_ERROR;

	try {

		const T & res = m_ulate.pop();
		*t = &res;

	} catch (end_of_stream_exception &) {
		retval = END_OF_STREAM;
	} catch (stream_exception &) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::peek(const T **t) {
    if(m_ulate.empty())
        return END_OF_STREAM;

    err retval = NO_ERROR;
	
	try {

		const T & res = m_ulate.top();
		*t = &res;

	} catch (end_of_stream_exception &) {
		retval = END_OF_STREAM;
	} catch (stream_exception &) {
		retval = IO_ERROR;
	}

	return retval;

}

/////////////////////////////////////////////////////////////////////////

template<class T>
err stack<T>::main_memory_usage(TPIE_OS_SIZE_T *usage,
								stream_usage usage_type) const {
    
    switch (usage_type) {

		//  All these types are o.k.
    case STREAM_USAGE_OVERHEAD:
    case STREAM_USAGE_CURRENT:
    case STREAM_USAGE_MAXIMUM:
    case STREAM_USAGE_SUBSTREAM:
    case STREAM_USAGE_BUFFER:
		*usage = tpie::stack<T>::memory_usage();
		*usage += sizeof(*this);
		break;

    default:
		tp_assert(0, "Unknown mem::stream_usage type added.");	
    }
    
    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////

} // ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_STACK_H 
