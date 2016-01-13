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

///////////////////////////////////////////////////////////////////////////////
// Graham sweep to compute the convex hull with a sort and two scans.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/pipelining.h>
#include <tpie/stack.h>
#include <tpie/file_stream.h>
#include <iostream>
#include <tpie/tempname.h>

namespace TP = tpie;
namespace P = tpie::pipelining;

template <typename T>
struct point {
	typedef T coord_type;
	T x;
	T y;

	bool operator!=(const point & other) const {
		return (x != other.x) || (y != other.y);
	}

	bool operator==(const point & other) const {
		return (x == other.x) && (y == other.y);
	}

	bool operator<(const point & other) const {
		return (x == other.x) ? (y < other.y) : (x < other.x);
	}
};

template <typename T>
bool left_turn(const point<T> & p1, const point<T> & p2, const point<T> & p3) {
	T sg = (p1.x-p3.x)*(p2.y-p3.y) - (p1.y-p3.y)*(p2.x-p3.x);
	return sg > 0;
}

template <typename T>
bool right_turn(const point<T> & p1, const point<T> & p2, const point<T> & p3) {
	return left_turn(p3, p2, p1);
}

// Second part of the computation. Reads the two stacks to construct the convex
// polygon. Outputs duplicates.
template <typename dest_t>
class graham_scan_reconstruct_type : public P::node {
	typedef typename dest_t::item_type Pt;

	TP::temp_file * uhTmp;

	/** Upper hull */
	TP::file_stream<Pt> * uh;

	/** Lower hull */
	TP::stack<Pt> * lh;

	dest_t dest;

public:
	typedef Pt item_type;

	graham_scan_reconstruct_type(dest_t dest)
		: dest(std::move(dest))
	{
		add_push_destination(dest);
		set_minimum_memory(sizeof(TP::temp_file)
						   + TP::stack<Pt>::memory_usage()
						   + TP::file_stream<Pt>::memory_usage());
	}

	void set_predecessor(P::node & pred) {
		add_dependency(pred);
	}

	virtual void begin() override {
		uhTmp = fetch<TP::temp_file *>("uhTmp");
		lh = fetch<TP::stack<Pt> *>("lh");
		uh = TP::tpie_new<TP::file_stream<Pt> >();
		uh->open(*uhTmp);

		std::cerr << "Upper hull: " << uh->size() << ", lower: " << lh->size() << std::endl;
		TP::stream_size_type items = uh->size() + lh->size();
		set_steps(items);
		forward<TP::stream_size_type>("items", items);
	}

	virtual void go() override {
		while (uh->can_read()) {
			dest.push(uh->read());
			step();
		}
		while (!lh->empty()) {
			dest.push(lh->pop());
			step();
		}
	}

	virtual void end() override {
		using TP::tpie_delete;
		tpie_delete(lh);
		tpie_delete(uh);
		tpie_delete(uhTmp);
	}
};

// First part of computation. Assumes input is sorted by x coordinate. Must be
// followed by a graham_scan_reconstruct_type instance.
template <typename dest_t>
class graham_scan_type;
template <typename rdest_t>
class graham_scan_type<graham_scan_reconstruct_type<rdest_t> > : public P::node {
	typedef graham_scan_reconstruct_type<rdest_t> dest_t;
	typedef typename dest_t::item_type Pt;
	typedef typename Pt::coord_type T;

	TP::temp_file * uhTmp;

	/** Upper hull */
	TP::stack<Pt> * uh;

	/** Lower hull */
	TP::stack<Pt> * lh;

	dest_t dest;

public:
	typedef Pt item_type;

	graham_scan_type(dest_t dest)
		: dest(std::move(dest))
	{
		this->dest.set_predecessor(*this);
		set_minimum_memory(sizeof(TP::temp_file)
						   + 2*TP::stack<Pt>::memory_usage());
	}

	virtual void begin() override {
		using TP::tpie_new;
		uhTmp = tpie_new<TP::temp_file>();
		uh = tpie_new<TP::stack<Pt> >(*uhTmp);
		lh = tpie_new<TP::stack<Pt> >();
		forward("uhTmp", uhTmp);
		forward("lh", lh);
	}

	void push(Pt pt) {
		if (uh->empty()) {
			std::clog << "Add first point." << std::endl;
			uh->push(pt);
			lh->push(pt);
		} else {
			add_upper(pt);
			add_lower(pt);
		}
	}

	virtual void end() override {
		TP::tpie_delete(uh);
	}

private:
	void add_upper(Pt pt) {
		add_hull(pt, uh, left_turn<T>);
	}

	void add_lower(Pt pt) {
		add_hull(pt, lh, right_turn<T>);
	}

	template <typename Pred>
	void add_hull(Pt pt, TP::stack<Pt> * stack, Pred pred) {
		Pt p2 = stack->top();
		if (p2 == pt) {
			// No need to have both points on stack.
			return;
		}
		stack->pop();
		Pt p1 = stack->empty() ? p2 : stack->pop();
		while (true) {
			if (pred(p1, p2, pt)) {
				if (!stack->empty()) {
					p2 = p1;
					p1 = stack->pop();
				} else {
					stack->push(p1);
					if (p1 != pt) {
						stack->push(pt);
					}
					break;
				}
			} else {
				stack->push(p1);
				stack->push(p2);
				stack->push(pt);
				break;
			}
		}
	}
};

typedef P::pipe_middle<P::factory<graham_scan_type> > graham_scan_in;


typedef P::pipe_middle<P::factory<graham_scan_reconstruct_type> > graham_scan_out;

// Aggregates two coordinates into one point.
template <typename dest_t>
class make_points_type : public P::node {
	typedef typename dest_t::item_type Pt;

	bool flag;
	Pt buffer;
	dest_t dest;
public:
	typedef typename Pt::coord_type item_type;

	make_points_type(dest_t dest)
		: flag(false)
		, dest(std::move(dest))
	{
		add_push_destination(dest);
	}

	void push(item_type coord) {
		if (!flag) {
			buffer.x = coord;
			flag = true;
		} else {
			buffer.y = coord;
			dest.push(buffer);
			flag = false;
		}
	}
};

typedef P::pipe_middle<P::factory<make_points_type> > make_points;

// Print and verify polygon. Weeds out duplicates.
template <typename T>
class print_points_type : public P::node {
	typedef point<T> Pt;
	int state;
	Pt buf[3];

	void invalid() {
		std::clog << "Not right turn" << std::endl;
	}

public:
	typedef Pt item_type;

	print_points_type() : state(3) {
	}

	void push(Pt pt) {
		std::cerr << pt.x << ' ' << pt.y << '\n';
		switch (state) {
			case 0:
				if (pt == buf[2]) return;
				buf[0] = pt;
				if (!right_turn(buf[1], buf[2], buf[0])) invalid();
				state = 1;
				break;
			case 1:
				if (pt == buf[0]) return;
				buf[1] = pt;
				if (!right_turn(buf[2], buf[0], buf[1])) invalid();
				state = 2;
				break;
			case 2:
				if (pt == buf[1]) return;
				buf[2] = pt;
				if (!right_turn(buf[0], buf[1], buf[2])) invalid();
				state = 0;
				break;
			case 3:
				buf[0] = pt;
				state = 4;
				break;
			case 4:
				if (pt == buf[0]) return;
				buf[1] = pt;
				state = 2;
				break;
		}
		std::cout << pt.x << ' ' << pt.y << '\n';
	}
};

template <typename T>
P::pipe_end<P::termfactory<print_points_type<T> > >
print_points() {
	return P::termfactory<print_points_type<T> >();
}

int main() {
	TP::tpie_init();
	TP::get_memory_manager().set_limit(50*1024*1024);
	{
		P::pipeline p
			= P::push_input_iterator(std::istream_iterator<int>(std::cin), std::istream_iterator<int>())
			| make_points()
			| P::sort()
			| graham_scan_in()
			| graham_scan_out()
			| print_points<int>();
		p.plot(std::clog);
		p();
	}
	TP::tpie_finish();
	return 0;
}
