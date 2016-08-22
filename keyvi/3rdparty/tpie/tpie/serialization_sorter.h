// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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

#ifndef TPIE_SERIALIZATION_SORTER_H
#define TPIE_SERIALIZATION_SORTER_H

#include <queue>
#include <boost/filesystem.hpp>

#include <tpie/array.h>
#include <tpie/array_view.h>
#include <tpie/tempname.h>
#include <tpie/tpie_log.h>
#include <tpie/stats.h>
#include <tpie/parallel_sort.h>

#include <tpie/serialization2.h>
#include <tpie/serialization_stream.h>

#include <tpie/pipelining/node.h>

namespace tpie {

namespace serialization_bits {

struct sort_parameters {
	/** files available while forming sorted runs. */
	memory_size_type filesPhase1;
	/** memory available while forming sorted runs. */
	memory_size_type memoryPhase1;
	/** files available while merging runs. */
	memory_size_type filesPhase2;
	/** Memory available while merging runs. */
	memory_size_type memoryPhase2;
	/** files available during output phase. */
	memory_size_type filesPhase3;
	/** Memory available during output phase. */
	memory_size_type memoryPhase3;
	/** Minimum size of serialized items. */
	memory_size_type minimumItemSize;
	/** Directory in which temporary files are stored. */
	std::string tempDir;

	void dump(std::ostream & out) const {
		out << "Serialization merge sort parameters\n"
			<< "Phase 1 files:               " << filesPhase1 << '\n'
			<< "Phase 1 memory:              " << memoryPhase1 << '\n'
			<< "Phase 2 files:               " << filesPhase2 << '\n'
			<< "Phase 2 memory:              " << memoryPhase2 << '\n'
			<< "Phase 3 files:               " << filesPhase3 << '\n'
			<< "Phase 3 memory:              " << memoryPhase3 << '\n'
			<< "Minimum item size:           " << minimumItemSize << '\n'
			<< "Temporary directory:         " << tempDir << '\n';
	}
};

template <typename T>
void set_owner(memory_bucket_ref b, T & item) {
	memory_size_type serSize = serialized_size(item);

	if (serSize > sizeof(T)) {
		// amount of memory this item needs for its extra stuff (stuff not in the buffer).
		serSize -= sizeof(T);
	}

	b->count += serSize;	
}

template <typename T>
void unset_owner(memory_bucket_ref /*b*/, T & /*item*/) {}

template <typename T, typename pred_t>
class internal_sort {
	array<T> m_buffer;
	memory_size_type m_items;
	memory_size_type m_memForItems;

	memory_size_type m_largestItem;

	pred_t m_pred;

	bool m_full;

	memory_bucket_ref m_buffer_bucket;
	memory_bucket_ref m_item_bucket;

public:
	internal_sort(memory_bucket_ref buffer_bucket, 
				  memory_bucket_ref item_bucket,
				  pred_t pred = pred_t())
		: m_buffer(buffer_bucket)
		, m_items(0)
		, m_largestItem(sizeof(T))
		, m_pred(pred)
		, m_full(false)
		, m_buffer_bucket(buffer_bucket)
		, m_item_bucket(item_bucket)
	{
	}

	void begin(memory_size_type memAvail) {
		m_buffer.resize(memAvail / sizeof(T) / 2);
		m_items = 0;
		m_largestItem = sizeof(T);
		m_full = false;
		m_memForItems = memAvail - m_buffer_bucket->count;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief True if all items up to and including this one fits in buffer.
	///
	/// Once push() returns false, it will keep returning false until
	/// the sequence is sorted, read out, and the buffer has been cleared.
	///////////////////////////////////////////////////////////////////////////
	bool push(const T & item) {
		if (m_full) return false;

		if (m_items == m_buffer.size()) {
			m_full = true;
			return false;
		}

		size_t oldSize = m_item_bucket->count;
		set_owner(m_item_bucket, item);

		if (m_item_bucket->count > m_memForItems) {
			unset_owner(m_item_bucket, item);
			m_full = true;
			return false;
		}

		m_largestItem = std::max(m_largestItem, m_item_bucket->count - oldSize);

		m_buffer[m_items++] = item;

		return true;
	}

	memory_size_type get_largest_item_size() {
		return m_largestItem;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get the serialized size of the items written.
	///
	/// This is exactly the size the current run will use when serialized to
	/// disk.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type current_serialized_size() {
		return m_item_bucket->count;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute current memory usage.
	///
	/// This includes the item buffer array as well as the extra serialized
	/// size of the items already written to the buffer.
	/// This assumes that items use as much primary memory as their serialized
	/// size. If this assumption does not hold, the memory usage reported may
	/// be useless. Nevertheless, this is the memory usage we use in our
	/// calculations.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type memory_usage() {
		return m_buffer_bucket->count + m_item_bucket->count;
	}

	bool can_shrink_buffer() {
		return current_serialized_size() <= get_memory_manager().available();
	}

	void shrink_buffer() {
		array<T> newBuffer(array_view<const T>(begin(), end()));
		m_buffer.swap(newBuffer);
	}

	void sort() {
		parallel_sort(m_buffer.get(), m_buffer.get() + m_items, m_pred);
	}

	const T * begin() const {
		return m_buffer.get();
	}

	const T * end() const {
		return m_buffer.get() + m_items;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Deallocate buffer and call reset().
	///////////////////////////////////////////////////////////////////////////
	void free() {
		reset();
		m_buffer.resize(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reset sorter, but keep the remembered largest item size and
	/// buffer size.
	///////////////////////////////////////////////////////////////////////////
	void reset() {
		for (size_t i = 0 ; i < m_items ; ++i)
			unset_owner(m_item_bucket, m_buffer[i]);
		m_item_bucket->count = 0;
		m_items = 0;
		m_full = false;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief  File handling for merge sort.
///
/// This class abstracts away the details of numbering run files; tracking the
/// number of runs in each merge level; informing the TPIE stats framework of
/// the temporary size; deleting run files after use.
///
/// The important part of the state is the tuple consisting of
/// (a, b, c) := (fileOffset, nextLevelFileOffset, nextFileOffset).
/// `a` is the first file in the level currently being merged;
/// `b` is the first file in the level being merged into;
/// `c` is the next file to write output to.
///
/// ## Transition system
///
/// We let remainingRuns := b - a, and nextLevelRuns := c - b.
///
/// The tuple (remainingRuns, nextLevelRuns) has the following transitions:
/// On open_new_writer(): (x, y) -> (x, 1+y),
/// On open_readers(fanout): (fanout+x, y) -> (fanout+x, y),
/// On open_readers(fanout): (0, fanout+y) -> (fanout+y, 0),
/// On close_readers_and_delete(): (fanout+x, y) -> (x, y).
///
/// ## Merge sorter usage
///
/// During run formation (the first phase of merge sort), we repeatedly call
/// open_new_writer() and close_writer() to write out runs to the disk.
///
/// After run formation, we call open_readers(fanout) to advance into the first
/// level of the merge heap (so one can think of run formation as a "zeroth
/// level" in the merge heap).
///
/// As a slight optimization, when remaining_runs() == 1, one may call
/// move_last_reader_to_next_level() to move the remaining run into the next
/// merge level without scanning through and copying the single remaining run.
///
/// See serialization_sorter::merge_runs() for the logic involving
/// next_level_runs() and remaining_runs() in a loop.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_handler {
	// Physical index of the run file with logical index 0.
	size_t m_fileOffset;
	// Physical index of the run file that begins the next run.
	size_t m_nextLevelFileOffset;
	// Physical index of the next run file to write
	size_t m_nextFileOffset;

	bool m_writerOpen;
	size_t m_readersOpen;

	serialization_writer m_writer;
	stream_size_type m_currentWriterByteSize;

	array<serialization_reader> m_readers;

	std::string m_tempDir;

	std::string run_file(size_t physicalIndex) {
		if (m_tempDir.size() == 0) throw exception("run_file: temp dir is the empty string");
		std::stringstream ss;
		ss << m_tempDir << '/' << physicalIndex << ".tpie";
		return ss.str();
	}

public:
	file_handler()
		: m_fileOffset(0)
		, m_nextLevelFileOffset(0)
		, m_nextFileOffset(0)

		, m_writerOpen(false)
		, m_readersOpen(0)

		, m_writer()
		, m_currentWriterByteSize(0)
	{
	}

	~file_handler() {
		reset();
	}

	void set_temp_dir(const std::string & tempDir) {
		if (m_nextFileOffset != 0)
			throw exception("set_temp_dir: trying to change path after files already open");
		m_tempDir = tempDir;
	}

	void open_new_writer() {
		if (m_writerOpen) throw exception("open_new_writer: Writer already open");
		m_writer.open(run_file(m_nextFileOffset++));
		m_currentWriterByteSize = m_writer.file_size();
		m_writerOpen = true;
	}

	void write(const T & v) {
		if (!m_writerOpen) throw exception("write: No writer open");
		m_writer.serialize(v);
	}

	void close_writer() {
		if (!m_writerOpen) throw exception("close_writer: No writer open");
		m_writer.close();
		stream_size_type sz = m_writer.file_size();
		increase_usage(m_nextFileOffset-1, static_cast<stream_offset_type>(sz));
		m_writerOpen = false;
	}

	size_t remaining_runs() {
		return m_nextLevelFileOffset - m_fileOffset;
	}

	size_t next_level_runs() {
		return m_nextFileOffset - m_nextLevelFileOffset;
	}

	bool readers_open() {
		return m_readersOpen > 0;
	}

	void open_readers(size_t fanout) {
		if (m_readersOpen != 0) throw exception("open_readers: readers already open");
		if (fanout == 0) throw exception("open_readers: fanout == 0");
		if (remaining_runs() == 0) {
			if (m_writerOpen) throw exception("Writer open while moving to next merge level");
			m_nextLevelFileOffset = m_nextFileOffset;
		}
		if (fanout > remaining_runs()) throw exception("open_readers: fanout out of bounds");

		if (m_readers.size() < fanout) m_readers.resize(fanout);
		for (size_t i = 0; i < fanout; ++i) {
			m_readers[i].open(run_file(m_fileOffset + i));
		}
		m_readersOpen = fanout;
	}

	bool can_read(size_t idx) {
		if (m_readersOpen == 0) throw exception("can_read: no readers open");
		if (m_readersOpen < idx) throw exception("can_read: index out of bounds");
		return m_readers[idx].can_read();
	}

	T read(size_t idx) {
		if (m_readersOpen == 0) throw exception("read: no readers open");
		if (m_readersOpen < idx) throw exception("read: index out of bounds");
		T res;
		m_readers[idx].unserialize(res);
		return res;
	}

	void close_readers_and_delete() {
		if (m_readersOpen == 0) throw exception("close_readers_and_delete: no readers open");

		for (size_t i = 0; i < m_readersOpen; ++i) {
			decrease_usage(m_fileOffset + i, m_readers[i].file_size());
			m_readers[i].close();
			boost::filesystem::remove(run_file(m_fileOffset + i));
		}
		m_fileOffset += m_readersOpen;
		m_readersOpen = 0;
	}

	void move_last_reader_to_next_level() {
		if (remaining_runs() != 1)
			throw exception("move_last_reader_to_next_level: remaining_runs != 1");
		m_nextLevelFileOffset = m_fileOffset;
	}

	void reset() {
		if (m_readersOpen > 0) {
			log_debug() << "reset: Close readers" << std::endl;
			close_readers_and_delete();
		}
		m_readers.resize(0);
		if (m_writerOpen) {
			log_debug() << "reset: Close writer" << std::endl;
			close_writer();
		}
		log_debug() << "Remove " << m_fileOffset << " through " << m_nextFileOffset << std::endl;
		for (size_t i = m_fileOffset; i < m_nextFileOffset; ++i) {
			std::string runFile = run_file(i);
			serialization_reader rd;
			rd.open(runFile);
			decrease_usage(i, rd.file_size());
			rd.close();
			boost::filesystem::remove(runFile);
		}
		m_fileOffset = m_nextLevelFileOffset = m_nextFileOffset = 0;
	}

private:
	void increase_usage(size_t idx, stream_size_type sz) {
		log_debug() << "+ " << idx << ' ' << sz << std::endl;
		increment_temp_file_usage(static_cast<stream_offset_type>(sz));
	}

	void decrease_usage(size_t idx, stream_size_type sz) {
		log_debug() << "- " << idx << ' ' << sz << std::endl;
		increment_temp_file_usage(-static_cast<stream_offset_type>(sz));
	}
};

template <typename T, typename pred_t>
class merger {
	class mergepred_t {
		pred_t m_pred;

	public:
		typedef std::pair<T, size_t> item_type;

		mergepred_t(const pred_t & pred) : m_pred(pred) {}

		// Used with std::priority_queue, so invert the original relation.
		bool operator()(const item_type & a, const item_type & b) const {
			return m_pred(b.first, a.first);
		}
	};

	typedef typename mergepred_t::item_type item_type;

	file_handler<T> & files;
	pred_t pred;
	std::vector<serialization_reader> rd;
	typedef std::priority_queue<item_type, std::vector<item_type>, mergepred_t> priority_queue_type;
	priority_queue_type pq;

public:
	merger(file_handler<T> & files, const pred_t & pred)
		: files(files)
		, pred(pred)
		, pq(mergepred_t(pred))
	{
	}

	// Assume files.open_readers(fanout) has just been called
	void init(size_t fanout) {
		rd.resize(fanout);
		for (size_t i = 0; i < fanout; ++i)
			push_from(i);
	}

	bool empty() const {
		return pq.empty();
	}

	const T & top() const {
		return pq.top().first;
	}

	void pop() {
		size_t idx = pq.top().second;
		pq.pop();
		push_from(idx);
	}

	// files.close_readers_and_delete() should be called after this
	void free() {
		{
			priority_queue_type tmp(pred);
			std::swap(pq, tmp);
		}
		rd.resize(0);
	}

private:
	void push_from(size_t idx) {
		if (files.can_read(idx)) {
			pq.push(std::make_pair(files.read(idx), idx));
		}
	}
};

} // namespace serialization_bits

template <typename T, typename pred_t = std::less<T> >
class serialization_sorter {
public:
	typedef std::shared_ptr<serialization_sorter> ptr;

private:
	enum sorter_state { state_initial, state_1, state_2, state_3 };

	std::unique_ptr<memory_bucket> m_buffer_bucket_ptr;
	memory_bucket_ref m_buffer_bucket;
	std::unique_ptr<memory_bucket> m_item_bucket_ptr;
	memory_bucket_ref m_item_bucket;
	pipelining::node * m_owning_node;

	sorter_state m_state;
	serialization_bits::internal_sort<T, pred_t> m_sorter;
	serialization_bits::sort_parameters m_params;
	bool m_parametersSet;
	serialization_bits::file_handler<T> m_files;
	serialization_bits::merger<T, pred_t> m_merger;

	stream_size_type m_items;
	bool m_reportInternal;
	const T * m_nextInternalItem;

	static const memory_size_type defaultFiles = 253; // Default number of files available, when not using set_available_files
	static const memory_size_type minimumFilesPhase1 = 1;
	static const memory_size_type maximumFilesPhase1 = 1;
	static const memory_size_type minimumFilesPhase2 = 3;
	static const memory_size_type maximumFilesPhase2 = std::numeric_limits<memory_size_type>::max();
	static const memory_size_type minimumFilesPhase3 = 3;
	static const memory_size_type maximumFilesPhase3 = std::numeric_limits<memory_size_type>::max();

	const int defaultMaxFiles = 253;

public:
	serialization_sorter(memory_size_type minimumItemSize = sizeof(T), pred_t pred = pred_t())
		: m_buffer_bucket_ptr(new memory_bucket())
		, m_buffer_bucket(memory_bucket_ref(m_buffer_bucket_ptr.get()))
		, m_item_bucket_ptr(new memory_bucket())
		, m_item_bucket(memory_bucket_ref(m_item_bucket_ptr.get()))
		, m_owning_node(nullptr)
		, m_state(state_initial)
		, m_sorter(m_buffer_bucket, m_item_bucket, pred)
		, m_parametersSet(false)
		, m_files()
		, m_merger(m_files, pred)
		, m_items(0)
		, m_reportInternal(false)
		, m_nextInternalItem(0)
	{
		m_params.filesPhase1 = 0;
		m_params.filesPhase2 = 0;
		m_params.filesPhase3 = 0;
		m_params.memoryPhase1 = 0;
		m_params.memoryPhase2 = 0;
		m_params.memoryPhase3 = 0;
		m_params.minimumItemSize = minimumItemSize;
	}

private:
	// Checks if we should still be able to change parameters
	inline void check_not_started() {
		if (m_state != state_initial) {
			throw tpie::exception("Can't change parameters after sorting has started");
		}
	}

public:
	inline void set_phase_1_files(memory_size_type f1) {
		m_params.filesPhase1 = f1;
		check_not_started();
	}

	inline void set_phase_2_files(memory_size_type f2) {
		m_params.filesPhase2 = f2;
		check_not_started();
	}

	inline void set_phase_3_files(memory_size_type f3) {
		m_params.filesPhase3 = f3;
		check_not_started();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given amount of files.
	/// \param f Files available for phase 1, 2 and 3
	///////////////////////////////////////////////////////////////////////////
	inline void set_available_files(memory_size_type f) {
		m_params.filesPhase1 = m_params.filesPhase2 = m_params.filesPhase3 = f;
		check_not_started();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate parameters from given amount of files.
	/// \param f1 Files available for phase 1
	/// \param f2 Files available for phase 2
	/// \param f3 Files available for phase 3
	///////////////////////////////////////////////////////////////////////////
	inline void set_available_files(memory_size_type f1, memory_size_type f2, memory_size_type f3) {
		m_params.filesPhase1 = f1;
		m_params.filesPhase2 = f2;
		m_params.filesPhase3 = f3;
		check_not_started();
	}

	void set_phase_1_memory(memory_size_type m1) {
		m_params.memoryPhase1 = m1;
		check_not_started();
	}

	void set_phase_2_memory(memory_size_type m2) {
		m_params.memoryPhase2 = m2;
		check_not_started();
	}

	void set_phase_3_memory(memory_size_type m3) {
		m_params.memoryPhase3 = m3;
		check_not_started();
	}

	void set_available_memory(memory_size_type m) {
		set_phase_1_memory(m);
		set_phase_2_memory(m);
		set_phase_3_memory(m);
	}

	void set_available_memory(memory_size_type m1, memory_size_type m2, memory_size_type m3) {
		set_phase_1_memory(m1);
		set_phase_2_memory(m2);
		set_phase_3_memory(m3);
	}

	static memory_size_type minimum_memory_phase_1() {
		return serialization_writer::memory_usage()*2;
	}

	static memory_size_type minimum_memory_phase_2() {
		return serialization_writer::memory_usage()
			+ 2*serialization_reader::memory_usage();
	}

	static memory_size_type minimum_memory_phase_3() {
		return 2*serialization_reader::memory_usage();
	}

	memory_size_type actual_memory_phase_3() {
		if (m_state != state_3)
			throw tpie::exception("Bad state in actualy_memory_phase_3");
		if (m_reportInternal)
			return m_sorter.memory_usage();
		else
			return m_files.next_level_runs() * (m_sorter.get_largest_item_size() + serialization_reader::memory_usage());
	}

	void set_owner(pipelining::node * n) {
		if (m_owning_node != nullptr) {
			m_buffer_bucket_ptr = std::move(m_owning_node->bucket(0));
			m_item_bucket_ptr = std::move(m_owning_node->bucket(1));
		}

		if (n != nullptr) {
			n->bucket(0) = std::move(m_buffer_bucket_ptr);
			n->bucket(1) = std::move(m_item_bucket_ptr);
		}

		m_owning_node = n;
	}
private:
	static memory_size_type clamp(memory_size_type lo, memory_size_type val, memory_size_type hi) {
		return std::max(lo, std::min(val, hi));
	}

	void calculate_parameters() {
		if (m_state != state_initial)
			throw tpie::exception("Bad state in calculate_parameters");

		if(!m_params.filesPhase1)
			m_params.filesPhase1 = clamp(minimumFilesPhase1, defaultFiles, maximumFilesPhase1);
		if(!m_params.filesPhase2)
			m_params.filesPhase2 = clamp(minimumFilesPhase2, defaultFiles, maximumFilesPhase2);
		if(!m_params.filesPhase3)
			m_params.filesPhase3 = clamp(minimumFilesPhase3, defaultFiles, maximumFilesPhase3);

		if(m_params.filesPhase1 < minimumFilesPhase1)
			throw tpie::exception("file limit for phase 1 too small (" + std::to_string(m_params.filesPhase1) + " < " + std::to_string(minimumFilesPhase1) + ")");
		if(m_params.filesPhase2 < minimumFilesPhase2)
			throw tpie::exception("file limit for phase 2 too small (" + std::to_string(m_params.filesPhase2) + " < " + std::to_string(minimumFilesPhase2) + ")");
		if(m_params.filesPhase3 < minimumFilesPhase3)
			throw tpie::exception("file limit for phase 3 too small (" + std::to_string(m_params.filesPhase3) + " < " + std::to_string(minimumFilesPhase3) + ")");

		memory_size_type memAvail1 = m_params.memoryPhase1;
		if (memAvail1 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for run formation; have " << memAvail1
				<< " bytes but " << serialization_writer::memory_usage()
				<< " is required for writing a run." << std::endl;
			throw exception("Not enough memory for run formation");
		}

		memory_size_type memAvail2 = m_params.memoryPhase2;

		// We have to keep a writer open no matter what.
		if (memAvail2 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << memAvail2
				<< ", writer usage = " << serialization_writer::memory_usage()
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		memory_size_type memAvail3 = m_params.memoryPhase3;

		// We have to keep a writer open no matter what.
		if (memAvail2 <= serialization_writer::memory_usage()) {
			log_error() << "Not enough memory for outputting. "
				<< "mem avail = " << memAvail3
				<< ", writer usage = " << serialization_writer::memory_usage()
				<< std::endl;
			throw exception("Not enough memory for outputting.");
		}

		memory_size_type memForMerge = std::min(memAvail2, memAvail3);

		// We do not yet know the serialized size of the largest item,
		// so this calculation has to be redone.
		// Instead, we assume that all items have minimum size.

		// We have to keep a writer open no matter what.
		memory_size_type fanoutMemory = memForMerge - serialization_writer::memory_usage();

		// This is a lower bound on the memory used per fanout.
		memory_size_type perFanout = m_params.minimumItemSize + serialization_reader::memory_usage();

		// Floored division to compute the largest possible fanout.
		memory_size_type fanout = std::min(fanoutMemory / perFanout, m_params.filesPhase2 - 1);
		if (fanout < 2) {
			log_error() << "Not enough memory for merging, even when minimum item size is assumed. "
				<< "mem avail = " << memForMerge
				<< ", fanout memory = " << fanoutMemory
				<< ", per fanout >= " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		m_params.tempDir = tempname::tpie_dir_name();
		m_files.set_temp_dir(m_params.tempDir);

		log_debug() << "Calculated serialization_sorter parameters.\n";
		m_params.dump(log_debug());
		log_debug() << std::flush;

		m_parametersSet = true;
	}

public:
	void begin() {
		if (!m_parametersSet)
			calculate_parameters();
		if (m_state != state_initial)
			throw tpie::exception("Bad state in begin");
		m_state = state_1;

		log_debug() << "Before begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		m_sorter.begin(m_params.memoryPhase1 - serialization_writer::memory_usage());
		log_debug() << "After internal sorter begin; mem usage = "
			<< get_memory_manager().used() << std::endl;
		boost::filesystem::create_directory(m_params.tempDir);
	}

	void push(const T & item) {
		if (m_state != state_1)
			throw tpie::exception("Bad state in push");

		++m_items;

		if (m_sorter.push(item)) return;
		end_run();
		if (!m_sorter.push(item)) {
			throw exception("Couldn't fit a single item in buffer");
		}
	}

	void end() {
		if (m_state != state_1)
			throw tpie::exception("Bad state in end");

		memory_size_type internalThreshold =
			std::min(m_params.memoryPhase2, m_params.memoryPhase3);

		log_debug() << "m_sorter.memory_usage == " << m_sorter.memory_usage() << '\n'
			<< "internalThreshold == " << internalThreshold << std::endl;

		if (m_items == 0) {
			m_reportInternal = true;
			m_nextInternalItem = 0;
			m_sorter.free();
			log_debug() << "Got no items. Internal reporting mode." << std::endl;
		} else if (m_files.next_level_runs() == 0
			&& m_sorter.memory_usage()
			   <= internalThreshold) {

			m_sorter.sort();
			m_reportInternal = true;
			m_nextInternalItem = m_sorter.begin();
			log_debug() << "Got " << m_sorter.current_serialized_size()
				<< " bytes of items. Internal reporting mode." << std::endl;
		} else if (m_files.next_level_runs() == 0
				   && m_sorter.current_serialized_size() <= internalThreshold
				   && m_sorter.can_shrink_buffer()) {

			m_sorter.sort();
			m_sorter.shrink_buffer();
			m_reportInternal = true;
			m_nextInternalItem = m_sorter.begin();
			log_debug() << "Got " << m_sorter.current_serialized_size()
				<< " bytes of items. Internal reporting mode after shrinking buffer." << std::endl;

		} else {

			end_run();
			log_debug() << "Got " << m_files.next_level_runs() << " runs. "
				<< "External reporting mode." << std::endl;
			m_sorter.free();
			m_reportInternal = false;
		}

		log_debug() << "After internal sorter end; mem usage = "
			<< get_memory_manager().used() << std::endl;

		m_state = state_2;
	}

	stream_size_type item_count() {
		return m_items;
	}

	void evacuate() {
		switch (m_state) {
			case state_initial:
				throw tpie::exception("Cannot evacuate in state initial");
			case state_1:
				throw tpie::exception("Cannot evacuate in state 1");
			case state_2:
			case state_3:
				if (m_reportInternal) {
					end_run();
					m_sorter.free();
					m_reportInternal = false;
					log_debug() << "Evacuate out of internal reporting mode." << std::endl;
				} else {
					log_debug() << "Evacuate in external reporting mode - noop." << std::endl;
				}
				break;
		}
	}

	memory_size_type evacuated_memory_usage() const {
		return 0;
	}


	bool is_merge_runs_free() {
		if (m_state != state_2)
			throw tpie::exception("Bad state in end");
		if (m_reportInternal) return true;

		memory_size_type largestItem = m_sorter.get_largest_item_size();
		memory_size_type fanoutMemory = m_params.memoryPhase2 - serialization_writer::memory_usage();
		memory_size_type perFanout = largestItem + serialization_reader::memory_usage();
		memory_size_type fanout = std::min(m_params.filesPhase2 - 1, fanoutMemory / perFanout);
		
		memory_size_type finalFanoutMemory = m_params.memoryPhase3;
		memory_size_type finalFanout = std::min(
				{m_params.filesPhase3 - 1, fanout, finalFanoutMemory / perFanout});

		return m_files.next_level_runs() <= finalFanout;
	}
	
	void merge_runs() {
		if (m_state != state_2)
			throw tpie::exception("Bad state in end");

		if (m_reportInternal) {
			log_debug() << "merge_runs: internal reporting; doing nothing." << std::endl;
			m_state = state_3;
			return;
		}

		memory_size_type largestItem = m_sorter.get_largest_item_size();
		if (largestItem == 0) {
			log_warning() << "Largest item is 0 bytes; doing nothing." << std::endl;
			m_state = state_3;
			return;
		}

		if (m_params.memoryPhase2 <= serialization_writer::memory_usage())
			throw exception("Not enough memory for merging.");

		// Perform almost the same computation as in calculate_parameters.
		// Only change the item size to largestItem rather than minimumItemSize.
		memory_size_type fanoutMemory = m_params.memoryPhase2 - serialization_writer::memory_usage();
		memory_size_type perFanout = largestItem + serialization_reader::memory_usage();
		memory_size_type fanout = std::min(fanoutMemory / perFanout, m_params.filesPhase2 - 1);

		if (fanout < 2) {
			log_error() << "Not enough memory for merging. "
				<< "mem avail = " << m_params.memoryPhase2
				<< ", fanout memory = " << fanoutMemory
				<< ", per fanout = " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		memory_size_type finalFanoutMemory = m_params.memoryPhase3;
		memory_size_type finalFanout = std::min(
				{m_params.filesPhase3 - 1, fanout, finalFanoutMemory / perFanout});

		if (finalFanout < 2) {
			log_error() << "Not enough memory for merging (final fanout < 2). "
				<< "mem avail = " << m_params.memoryPhase3
				<< ", final fanout memory = " << finalFanoutMemory
				<< ", per fanout = " << perFanout
				<< std::endl;
			throw exception("Not enough memory for merging.");
		}

		log_debug() << "Calculated merge phase parameters for serialization sort.\n"
			<< "Fanout:       " << fanout << '\n'
			<< "Final fanout: " << finalFanout << '\n'
			;

		while (m_files.next_level_runs() > finalFanout) {
			if (m_files.remaining_runs() != 0)
				throw exception("m_files.remaining_runs() != 0");
			log_debug() << "Runs in current level: " << m_files.next_level_runs() << '\n';
			for (size_t remainingRuns = m_files.next_level_runs(); remainingRuns > 0;) {
				size_t f = std::min(fanout, remainingRuns);
				merge_runs(f);
				remainingRuns -= f;
				if (remainingRuns != m_files.remaining_runs())
					throw exception("remainingRuns != m_files.remaining_runs()");
			}
		}

		m_state = state_3;
	}

private:
	void end_run() {
		m_sorter.sort();
		if (m_sorter.begin() == m_sorter.end()) return;
		m_files.open_new_writer();
		for (const T * item = m_sorter.begin(); item != m_sorter.end(); ++item) {
			m_files.write(*item);
		}
		m_files.close_writer();
		m_sorter.reset();
	}

	void initialize_merger(size_t fanout) {
		if (fanout == 0) throw exception("initialize_merger: fanout == 0");
		m_files.open_readers(fanout);
		m_merger.init(fanout);
	}

	void free_merger_and_files() {
		m_merger.free();
		m_files.close_readers_and_delete();
	}

	void merge_runs(size_t fanout) {
		if (fanout == 0) throw exception("merge_runs: fanout == 0");

		if (fanout == 1 && m_files.remaining_runs() == 1) {
			m_files.move_last_reader_to_next_level();
			return;
		}

		initialize_merger(fanout);
		m_files.open_new_writer();
		while (!m_merger.empty()) {
			m_files.write(m_merger.top());
			m_merger.pop();
		}
		free_merger_and_files();
		m_files.close_writer();
	}

public:
	T pull() {
		if (!can_pull())
			throw exception("pull: !can_pull");

		if (m_reportInternal) {
			T item = *m_nextInternalItem++;
			if (m_nextInternalItem == m_sorter.end()) {
				m_sorter.free();
				m_nextInternalItem = 0;
			}
			return item;
		}

		if (!m_files.readers_open()) {
			if (m_files.next_level_runs() == 0)
				throw exception("pull: next_level_runs == 0");
			initialize_merger(m_files.next_level_runs());
		}

		T item = m_merger.top();
		m_merger.pop();

		if (m_merger.empty()) {
			free_merger_and_files();
			m_files.reset();
		}

		return item;
	}

	bool can_pull() {
		if (m_reportInternal) return m_nextInternalItem != 0;
		if (!m_files.readers_open()) return m_files.next_level_runs() > 0;
		return !m_merger.empty();
	}
};

}

#endif // TPIE_SERIALIZATION_SORTER_H
