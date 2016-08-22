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

#ifndef __TPIE_PIPELINING_HELPERS_H__
#define __TPIE_PIPELINING_HELPERS_H__

#include <iostream>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/memory.h>
#include <tpie/tpie_assert.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename dest_t>
class ostream_logger_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	inline ostream_logger_t(dest_t dest, std::ostream & log) : dest(std::move(dest)), log(log), begun(false), ended(false) {
		add_push_destination(this->dest);
		set_name("Log", PRIORITY_INSIGNIFICANT);
	}
	virtual void begin() override {
		node::begin();
		begun = true;
	}
	virtual void end() override {
		node::end();
		ended = true;
	}
	inline void push(const item_type & item) {
		if (!begun) {
			log << "WARNING: push() called before begin(). Calling begin on rest of pipeline." << std::endl;
			begin();
		}
		if (ended) {
			log << "WARNING: push() called after end()." << std::endl;
			ended = false;
		}
		log << "pushing " << item << std::endl;
		dest.push(item);
	}
private:
	dest_t dest;
	std::ostream & log;
	bool begun;
	bool ended;
};

template <typename source_t>
class pull_peek_t : public node {
public:
	typedef typename pull_type<source_t>::type item_type;

	pull_peek_t(source_t source) : source(std::move(source)) {
		add_pull_source(this->source);
		set_plot_options(PLOT_SIMPLIFIED_HIDE);
	}

	virtual void begin() override {
		could_pull = source.can_pull();
		if (could_pull) item=source.pull();
	}

	item_type pull() {
		item_type i = item;
		could_pull = source.can_pull();
		if (could_pull) item=source.pull();
		return i;
	}

	const item_type & peek() const {
		return item;
	}

	bool can_pull() const {
		return could_pull;
	}

private:
	item_type item;
	bool could_pull;
	source_t source;
};

template <typename T>
class dummydest_t : public node {
public:
	dummydest_t() : buffer(std::make_shared<T>()) {}

	typedef T item_type;
	std::shared_ptr<T> buffer;
	inline void push(const T & el) {
		*buffer = el;
	}
	inline T pull() {
		return *buffer;
	}
};

template <typename fact2_t>
class fork_t {
public:
	typedef typename fact2_t::constructed_type dest2_t;

	template <typename dest_t>
	class type : public node {
	public:
		typedef typename push_type<dest_t>::type item_type;

		type(dest_t dest, fact2_t fact2) : dest(std::move(dest)), dest2(fact2.construct()) {
			add_push_destination(this->dest);
			add_push_destination(dest2);
		}

		inline void push(const item_type & item) {
			dest.push(item);
			dest2.push(item);
		}

	private:
		dest_t dest;
		dest2_t dest2;
	};
};


template <typename source_t, typename dest_fact_t>
class pull_fork_t: public node {
public:
	typedef typename pull_type<source_t>::type item_type;

	pull_fork_t(source_t source, dest_fact_t dest_fact)
		: dest(dest_fact.construct())
		, source(std::move(source)) {
		add_pull_source(this->source);
		add_push_destination(dest);
	}
	
	bool can_pull() {return source.can_pull();}
	
	item_type pull() {
		item_type i=source.pull();
		dest.push(i);
		return i;
	}

private:
	typename dest_fact_t::constructed_type dest;
	source_t source;
};


template <typename T>
class null_sink_t: public node {
public:
	typedef T item_type;

	void push(const T &) {}
};


template <typename T>
class zero_source_t: public node {
public:
	typedef T item_type;

	T pull() {tpie_unreachable();}
	bool can_pull() {return false;}
};


template <typename IT>
class pull_input_iterator_t: public node {
	IT i;
	IT till;
public:
	typedef typename IT::value_type item_type;
	pull_input_iterator_t(IT from, IT to)
		: i(from)
		, till(to)
	{
	}

	bool can_pull() {
		return i != till;
	}

	item_type pull() {
		return *i++;
	}
};

template <typename IT>
class push_input_iterator_t {
public:
	template <typename dest_t>
	class type : public node {
		IT i;
		IT till;
		dest_t dest;
	public:
		type(dest_t dest, IT from, IT to)
			: i(from)
			, till(to)
			, dest(std::move(dest))
		{
			add_push_destination(dest);
		}

		virtual void go() override {
			while (i != till) {
				dest.push(*i);
				++i;
			}
		}
	};
};

template <typename Iterator, typename Item = void>
class push_output_iterator_t;

template <typename Iterator>
class push_output_iterator_t<Iterator, void> : public node {
	Iterator i;
public:
	typedef typename Iterator::value_type item_type;
	push_output_iterator_t(Iterator to)
		: i(to)
	{
	}

	void push(const item_type & item) {
		*i = item;
		++i;
	}
};

template <typename Iterator, typename Item>
class push_output_iterator_t : public node {
	Iterator i;
public:
	typedef Item item_type;
	push_output_iterator_t(Iterator to)
		: i(to)
	{
	}

	void push(const item_type & item) {
		*i = item;
		++i;
	}
};

template <typename IT>
class pull_output_iterator_t {
public:
	template <typename dest_t>
	class type : public node {
		IT i;
		dest_t dest;
	public:
		type(dest_t dest, IT to)
			: i(to)
			, dest(std::move(dest))
		{
			add_pull_source(dest);
		}

		virtual void go() override {
			while (dest.can_pull()) {
				*i = dest.pull();
				++i;
			}
		}
	};
};

template <typename F>
class preparer_t {
public:
	template <typename dest_t>
	class type: public node {
	private:
		F functor;
		dest_t dest;
	public:
		typedef typename push_type<dest_t>::type item_type;
		type(dest_t dest, const F & functor): functor(functor), dest(std::move(dest)) {
			add_push_destination(this->dest);
		}

		void prepare() override {
			functor(*static_cast<node*>(this));
		};

		void push(const item_type & item) {dest.push(item);}
	};
};


template <typename F>
class propagater_t {
public:
	template <typename dest_t>
	class type: public node {
	private:
		F functor;
		dest_t dest;
	public:
		typedef typename push_type<dest_t>::type item_type;
		type(dest_t dest, const F & functor): functor(functor), dest(std::move(dest)) {
			add_push_destination(this->dest);
		}

		void propagate() override {
			functor(*static_cast<node*>(this));
		};

		void push(const item_type & item) {dest.push(item);}
	};
};

template <typename src_fact_t>
struct zip_t {
	typedef typename src_fact_t::constructed_type src_t;
	template <typename dest_t>
	class type: public node {
	public:
		typedef typename push_type<dest_t>::type::first_type item_type;
		
		type(dest_t dest, src_fact_t src_fact)
			: src(src_fact.construct()), dest(std::move(dest)) {
			add_push_destination(this->dest);
			add_pull_source(src);
		}

		void push(const item_type & item) {
			tp_assert(src.can_pull(), "We should be able to pull");
			dest.push(std::make_pair(item, src.pull()));
		}
	private:
		src_t src;
		dest_t dest;
	};
};

template <typename fact2_t>
struct unzip_t {
	typedef typename fact2_t::constructed_type dest2_t;
	
	template <typename dest1_t> 
	class type: public node {
	public:
		typedef typename push_type<dest1_t>::type first_type;
		typedef typename push_type<dest2_t>::type second_type;
		typedef std::pair<first_type, second_type> item_type;
		
		type(dest1_t dest1, fact2_t fact2) : dest1(std::move(dest1)), dest2(fact2.construct()) {
			add_push_destination(this->dest1);
			add_push_destination(dest2);
		}
		
		void push(const item_type & item) {
			dest2.push(item.second);
			dest1.push(item.first);
		}
	private:
		dest1_t dest1;
		dest2_t dest2;
	};
};

template <typename T>
class item_type_t {
public:
	template <typename dest_t>
	class type: public node {
	private:
		dest_t dest;
	public:
		typedef T item_type;
		type(dest_t dest): dest(std::move(dest)) {}
		void push(const item_type & item) {dest.push(item);}
	};
};

template <typename fact_t>
class pull_source_t {
public:
	template <typename dest_t>
	class type: public node {
	public:
		typedef typename fact_t::constructed_type source_t;
		typedef typename push_type<dest_t>::type item_type;

		type(dest_t dest, fact_t fact)
			: dest(std::move(dest)), src(fact.construct()) {
			add_push_destination(dest);
			add_pull_source(src);
		}

		void propagate() override {
			size_t size = fetch<stream_size_type>("items");
			set_steps(size);
		}

		void go() override {
			while (src.can_pull()) {
				dest.push(src.pull());
				step();
			}
		}

	private:
		dest_t dest;
		source_t src;
	};
};

template <typename dest_t, typename equal_t>
class unique_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	unique_t(dest_t dest, equal_t equal)
		: equal(equal), dest(std::move(dest)) {}

	void begin() override {
		first = true;
	}

	void push(const item_type & item) {
		if (!first && equal(prev, item))
			return;
		first = false;
		dest.push(item);
		prev = item;
	}

private:
	bool first;
	equal_t equal;
	item_type prev;
	dest_t dest;
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes items to standard out and then pushes
/// them to the next node.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<factory<bits::ostream_logger_t, std::ostream &> >
cout_logger() {
	return {std::cout};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that allows peeking at the next item in the pipeline
///////////////////////////////////////////////////////////////////////////////
typedef pullpipe_middle<factory<bits::pull_peek_t> > pull_peek;


///////////////////////////////////////////////////////////////////////////////
/// \brief Create a fork pipe node.
/// 
/// Whenever an element e is push into the fork node, e is pushed
/// to the destination and then to "to"
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_middle<tempfactory<bits::fork_t<fact_t>, fact_t> >
fork(pipe_end<fact_t> to) {
	return {std::move(to.factory)};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a pulling fork pipe node.
/// 
/// Whenever an element e is pulled from fork node, e is first pushed
/// into the destination
///////////////////////////////////////////////////////////////////////////////
template <typename dest_fact_t>
pullpipe_middle<tfactory<bits::pull_fork_t, Args<dest_fact_t>, dest_fact_t> >
pull_fork(dest_fact_t dest_fact) {
	return {std::move(dest_fact)};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create unzip pipe node.
/// 
/// Whenever a std::pair<A,B>(a,b) is pushed to the unzip node,
/// a is pushed to its destination, and then b is pushed to "to"
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_middle<tempfactory<bits::unzip_t<fact_t>, fact_t> >
unzip(pipe_end<fact_t> to) {
	return {std::move(to.factory)};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a zip pipe node.
///
/// Whenever an element a is pushed to the zip node,
/// an element b is pulled from the "from" node, 
/// and std::make_pair(a,b) is pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_middle<tempfactory<bits::zip_t<fact_t>, fact_t> >
zip(pullpipe_begin<fact_t> from) {
	return {std::move(from.factory)};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a dummy end pipe node
///
/// Whenever an element of type T is pushed to the null_sink it is disregarded
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory<bits::null_sink_t<T> > >
null_sink() {return termfactory<bits::null_sink_t<T> >();}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a dummy pull begin pipe node
///
/// Whenever an element of type T is pushed to the null_sink it is disregarded
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pullpipe_begin<termfactory<bits::zero_source_t<T> > >
zero_source() {return termfactory<bits::zero_source_t<T> >();}


template <template <typename dest_t> class Fact, typename... T>
pipe_begin<factory<Fact, T...> > make_pipe_begin(T... t) {
	return {t...};
}

template <template <typename dest_t> class Fact, typename... T>
pipe_middle<factory<Fact, T...> > make_pipe_middle(T... t) {
	return {t...};
}

template <typename Fact, typename... T>
pipe_end<termfactory<Fact, T...> > make_pipe_end(T ... t) {
	return {t...};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe that returns items in the range given by two iterators
/// \param begin The iterator pointing to the first item
/// \param end The iterator pointing to the end of the range
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pullpipe_begin<termfactory<bits::pull_input_iterator_t<IT>, IT, IT> > pull_input_iterator(IT begin, IT end) {
	return termfactory<bits::pull_input_iterator_t<IT>, IT, IT>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that pushes the items in the range given by
/// two iterators
/// \param begin The iterator pointing to the first item
/// \param end The iterator pointing to the end of the range
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pipe_begin<tempfactory<bits::push_input_iterator_t<IT>, IT, IT> > push_input_iterator(IT begin, IT end) {
	return tempfactory<bits::push_input_iterator_t<IT>, IT, IT>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that writes its given items to the destination pointed to by
/// the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pipe_end<termfactory<bits::push_output_iterator_t<IT>, IT> > push_output_iterator(IT to) {
	return termfactory<bits::push_output_iterator_t<IT>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that writes its given items to the destination pointed to by
/// the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
/// \tparam Item The type of the pushed items
///////////////////////////////////////////////////////////////////////////////
template <typename Item, typename IT>
pipe_end<termfactory<bits::push_output_iterator_t<IT, Item>, IT> > typed_push_output_iterator(IT to) {
	return termfactory<bits::push_output_iterator_t<IT, Item>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that writes its given items to the destination
/// pointed to by the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pullpipe_end<tempfactory<bits::pull_output_iterator_t<IT>, IT> > pull_output_iterator(IT to) {
	return tempfactory<bits::pull_output_iterator_t<IT>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create preparer callback identity pipe node
///
/// When prepare is called on the node the functor is called
/// Whenever an element is pushed, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory<bits::preparer_t<F>, F> > preparer(const F & functor) {
	return tempfactory<bits::preparer_t<F>, F>(functor);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create propagate callback identity pipe node
///
/// When propagate is called on the node the functor is called
/// Whenever an element is pushed, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory<bits::propagater_t<F>, F> > propagater(const F & functor) {
	return tempfactory<bits::propagater_t<F>, F>(functor);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create item type defining identity pipe node
///
/// Defines the item_type to be T.
/// Whenever an element is push, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename T>
pipe_middle<tempfactory<bits::item_type_t<T> > > item_type() {
	return tempfactory<bits::item_type_t<T> >();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that pulls items from source and push them into dest
///
/// \param The pull source, and the source forwards the number of items, "items"
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
pipe_begin<tempfactory<bits::pull_source_t<fact_t>, fact_t> >
pull_source(pullpipe_begin<fact_t> from) {
	return {std::move(from.factory)};
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Filter consecutive duplicates out
///
/// When items are pushed in 
/// Whenever a pushed itme is same as the previous, it is dropped
///////////////////////////////////////////////////////////////////////////////
template <typename equal_t>
pipe_middle<tfactory<bits::unique_t, Args<equal_t>, equal_t> > unique(equal_t equal) {
	return {equal};
}

}

}

#endif
