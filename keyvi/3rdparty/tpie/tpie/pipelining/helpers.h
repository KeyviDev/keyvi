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

	inline ostream_logger_t(TPIE_TRANSFERABLE(dest_t) dest, std::ostream & log) : dest(TPIE_MOVE(dest)), log(log), begun(false), ended(false) {
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

template <typename dest_t>
class identity_t : public node {
public:
	typedef typename push_type<dest_t>::type item_type;

	inline identity_t(TPIE_TRANSFERABLE(dest_t) dest) : dest(TPIE_MOVE(dest)) {
		add_push_destination(this->dest);
	}

	inline void push(const item_type & item) {
		dest.push(item);
	}
private:
	dest_t dest;
};

template <typename source_t>
class pull_identity_t : public node {
public:
	typedef typename pull_type<source_t>::type item_type;

	inline pull_identity_t(TPIE_TRANSFERABLE(source_t) source) : source(TPIE_MOVE(source)) {
		add_pull_source(this->source);
		set_name("Identity", PRIORITY_INSIGNIFICANT);
	}

	inline item_type pull() {
		return source.pull();
	}

	inline bool can_pull() {
		return source.can_pull();
	}

private:
	source_t source;
};

template <typename source_t>
class pull_peek_t : public node {
public:
	typedef typename pull_type<source_t>::type item_type;

	pull_peek_t(TPIE_TRANSFERABLE(source_t) source) : source(TPIE_MOVE(source)) {
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
	dummydest_t() : buffer(new T()) {}

	typedef T item_type;
	boost::shared_ptr<T> buffer;
	inline void push(const T & el) {
		*buffer = el;
	}
	inline T pull() {
		return *buffer;
	}
};

template <typename pushfact_t>
class push_to_pull {
public:

	template <typename source_t>
	class puller_t : public node {
	public:

		typedef typename pull_type<source_t>::type item_type;
		typedef typename pushfact_t::template constructed<dummydest_t<item_type> >::type pusher_t;

		source_t source;
		dummydest_t<item_type> dummydest;
		pusher_t pusher;

		inline puller_t(TPIE_TRANSFERABLE(source_t) source, const pushfact_t & pushfact)
			: source(TPIE_MOVE(source))
			, pusher(pushfact.construct(dummydest_t<item_type>(dummydest)))
		{
			add_pull_source(this->source);
			add_push_destination(pusher);
		}

		inline item_type pull() {
			pusher.push(source.pull());
			return dummydest.pull();
		}

		inline bool can_pull() {
			return source.can_pull();
		}

	};
};

template <typename pullfact_t>
class pull_to_push {
public:

	template <typename dest_t>
	class pusher_t : public node {
	public:
		typedef typename push_type<dest_t>::type item_type;
		typedef typename pullfact_t::template constructed<dummydest_t<item_type> >::type puller_t;

		dest_t dest;
		dummydest_t<item_type> dummydest;
		puller_t puller;

		inline pusher_t(TPIE_TRANSFERABLE(dest_t) dest, const pullfact_t & pullfact)
			: dest(TPIE_MOVE(dest))
			, puller(pullfact.construct(dummydest_t<item_type>(dummydest)))
		{
			add_push_destination(this->dest);
			add_pull_source(puller);
		}

		inline void push(const item_type & item) {
			dummydest.push(item);
			dest.push(puller.pull());
		}

	};
};


template <typename fact2_t>
class fork_t {
public:
	typedef typename fact2_t::constructed_type dest2_t;

	template <typename dest_t>
	class type : public node {
	public:
		typedef typename push_type<dest_t>::type item_type;

		inline type(TPIE_TRANSFERABLE(dest_t) dest, const fact2_t & fact2) : dest(TPIE_MOVE(dest)), dest2(fact2.construct()) {
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
			, dest(dest)
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
			, dest(dest)
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
		type(TPIE_TRANSFERABLE(dest_t) dest, const F & functor): functor(functor), dest(TPIE_MOVE(dest)) {
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
		type(TPIE_TRANSFERABLE(dest_t) dest, const F & functor): functor(functor), dest(TPIE_MOVE(dest)) {
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
		
		type(TPIE_TRANSFERABLE(dest_t) dest, const src_fact_t & src_fact)
			: src(src_fact.construct()), dest(TPIE_MOVE(dest)) {
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
		
		type(TPIE_TRANSFERABLE(dest1_t) dest1, const fact2_t & fact2) : dest1(TPIE_MOVE(dest1)), dest2(fact2.construct()) {
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
		type(TPIE_TRANSFERABLE(dest_t) dest): dest(TPIE_MOVE(dest)) {}
		void push(const item_type & item) {dest.push(item);}
	};
};

} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that writes items to standard out and then pushes
/// them to the next node.
///////////////////////////////////////////////////////////////////////////////
inline pipe_middle<factory_1<bits::ostream_logger_t, std::ostream &> >
cout_logger() {
	return factory_1<bits::ostream_logger_t, std::ostream &>(std::cout);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief The identity node does nothing but push the items to the next node
///////////////////////////////////////////////////////////////////////////////
typedef pipe_middle<factory_0<bits::identity_t> > identity;

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that does nothing to its items
///////////////////////////////////////////////////////////////////////////////
inline pullpipe_middle<factory_1<bits::push_to_pull<factory_0<bits::identity_t> >::puller_t, factory_0<bits::identity_t> > > pull_identity() {
	return factory_1<bits::push_to_pull<factory_0<bits::identity_t> >::puller_t, factory_0<bits::identity_t> >(factory_0<bits::identity_t>());
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that allows peeking at the next item in the pipeline
///////////////////////////////////////////////////////////////////////////////
typedef pullpipe_middle<factory_0<bits::pull_peek_t> > pull_peek;

inline
pipe_middle<factory_1<
	bits::pull_to_push<factory_0<bits::pull_identity_t> >::pusher_t,
	factory_0<bits::pull_identity_t>
> >
alt_identity() {
	return factory_1<
		bits::pull_to_push<factory_0<bits::pull_identity_t> >::pusher_t,
		factory_0<bits::pull_identity_t>
	>(factory_0<bits::pull_identity_t>());
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a fork pipe node.
/// 
/// Whenever an element e is push into the fork node, e is pushed
/// to the destination and then to "to"
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
inline pipe_middle<tempfactory_1<bits::fork_t<fact_t>, const fact_t &> >
fork(const pipe_end<fact_t> & to) {
	return tempfactory_1<bits::fork_t<fact_t>, const fact_t &>(to.factory);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create unzip pipe node.
/// 
/// Whenever a std::pair<A,B>(a,b) is pushed to the unzip node,
/// a is pushed to its destination, and then b is pushed to "to"
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
inline pipe_middle<tempfactory_1<bits::unzip_t<fact_t>, const fact_t &> >
unzip(const pipe_end<fact_t> & to) {
	return tempfactory_1<bits::unzip_t<fact_t>, const fact_t &>(to.factory);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a zip pipe node.
///
/// Whenever an element a is pushed to the zip node,
/// an element b is pulled from the "from" node, 
/// and std::make_pair(a,b) is pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
inline pipe_middle<tempfactory_1<bits::zip_t<fact_t>, const fact_t &> >
zip(const pullpipe_begin<fact_t> & from) {
	return tempfactory_1<bits::zip_t<fact_t>, const fact_t &>(from.factory);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a dummy end pipe node
///
/// Whenever an element of type T is pushed to the null_sink it is disregarded
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory_0<bits::null_sink_t<T> > >
null_sink() {return termfactory_0<bits::null_sink_t<T> >();}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a dummy pull begin pipe node
///
/// Whenever an element of type T is pushed to the null_sink it is disregarded
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pullpipe_begin<termfactory_0<bits::zero_source_t<T> > >
zero_source() {return termfactory_0<bits::zero_source_t<T> >();}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a dummy end pipe node
///
/// \deprecated{Use null_sink}
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline pipe_end<termfactory_0<bits::null_sink_t<T> > >
bitbucket(T) {
	return termfactory_0<bits::null_sink_t<T> >();
}

template <template <typename dest_t> class Fact>
pipe_begin<factory_0<Fact> > make_pipe_begin_0() {
	return pipe_begin<factory_0<Fact> >(factory_0<Fact>());
}

template <template <typename dest_t> class Fact>
pipe_middle<factory_0<Fact> > make_pipe_middle_0() {
	return pipe_middle<factory_0<Fact> >(factory_0<Fact>());
}

template <typename Fact>
pipe_end<termfactory_0<Fact> > make_pipe_end_0() {
	return pipe_end<termfactory_0<Fact> >(termfactory_0<Fact>());
}

template <template <typename dest_t> class Fact, typename T1>
pipe_begin<factory_1<Fact, T1> > make_pipe_begin_1(T1 e1) {
	return pipe_begin<factory_1<Fact, T1> >(factory_1<Fact, T1>(e1));
}

template <template <typename dest_t> class Fact, typename T1>
pipe_middle<factory_1<Fact, T1> > make_pipe_middle_1(T1 e1) {
	return pipe_middle<factory_1<Fact, T1> >(factory_1<Fact, T1>(e1));
}

template <typename Fact, typename T1>
pipe_end<termfactory_1<Fact, T1> > make_pipe_end_1(T1 e1) {
	return pipe_end<termfactory_1<Fact, T1> >(termfactory_1<Fact, T1>(e1));
}

template <template <typename dest_t> class Fact, typename T1, typename T2>
pipe_begin<factory_2<Fact, T1, T2> > make_pipe_begin_2(T1 e1, T2 e2) {
	return pipe_begin<factory_2<Fact, T1, T2> >(factory_2<Fact, T1, T2>(e1, e2));
}

template <template <typename dest_t> class Fact, typename T1, typename T2>
pipe_middle<factory_2<Fact, T1, T2> > make_pipe_middle_2(T1 e1, T2 e2) {
	return pipe_middle<factory_2<Fact, T1, T2> >(factory_2<Fact, T1, T2>(e1, e2));
}

template <typename Fact, typename T1, typename T2>
pipe_end<termfactory_2<Fact, T1, T2> > make_pipe_end_2(T1 e1, T2 e2) {
	return pipe_end<termfactory_2<Fact, T1, T2> >(termfactory_2<Fact, T1, T2>(e1, e2));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe that returns items in the range given by two iterators
/// \param begin The iterator pointing to the first item
/// \param end The iterator pointing to the end of the range
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pullpipe_begin<termfactory_2<bits::pull_input_iterator_t<IT>, IT, IT> > pull_input_iterator(IT begin, IT end) {
	return termfactory_2<bits::pull_input_iterator_t<IT>, IT, IT>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that pushes the items in the range given by
/// two iterators
/// \param begin The iterator pointing to the first item
/// \param end The iterator pointing to the end of the range
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pipe_begin<tempfactory_2<bits::push_input_iterator_t<IT>, IT, IT> > push_input_iterator(IT begin, IT end) {
	return tempfactory_2<bits::push_input_iterator_t<IT>, IT, IT>(begin, end);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that writes its given items to the destination pointed to by
/// the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pipe_end<termfactory_1<bits::push_output_iterator_t<IT>, IT> > push_output_iterator(IT to) {
	return termfactory_1<bits::push_output_iterator_t<IT>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A node that writes its given items to the destination pointed to by
/// the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
/// \tparam Item The type of the pushed items
///////////////////////////////////////////////////////////////////////////////
template <typename Item, typename IT>
pipe_end<termfactory_1<bits::push_output_iterator_t<IT, Item>, IT> > typed_push_output_iterator(IT to) {
	return termfactory_1<bits::push_output_iterator_t<IT, Item>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief A pull-pipe node that writes its given items to the destination
/// pointed to by the given iterator
/// \param to An iterator pointing to the destination that items should be
/// written to
///////////////////////////////////////////////////////////////////////////////
template <typename IT>
pullpipe_end<tempfactory_1<bits::pull_output_iterator_t<IT>, IT> > pull_output_iterator(IT to) {
	return tempfactory_1<bits::pull_output_iterator_t<IT>, IT>(to);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create preparer callback identity pipe node
///
/// When prepare is called on the node the functor is called
/// Whenever an element is pushed, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory_1<bits::preparer_t<F>, F> > preparer(const F & functor) {
	return tempfactory_1<bits::preparer_t<F>, F>(functor);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create propagate callback identity pipe node
///
/// When propagate is called on the node the functor is called
/// Whenever an element is pushed, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename F>
pipe_middle<tempfactory_1<bits::propagater_t<F>, F> > propagater(const F & functor) {
	return tempfactory_1<bits::propagater_t<F>, F>(functor);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Create item type defining identity pipe node
///
/// Defines the item_type to be T.
/// Whenever an element is push, it is immidiately pushed to the destination
///////////////////////////////////////////////////////////////////////////////
template <typename T>
pipe_middle<tempfactory_0<bits::item_type_t<T> > > item_type() {
	return tempfactory_0<bits::item_type_t<T> >();
}


}

}

#endif
