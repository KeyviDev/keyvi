// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
// Copyright 2014 The TPIE development team
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

#include "common.h"
#include <tpie/pipelining/node_name.h>

class node_name_tester {
public:
	node_name_tester(tpie::teststream & ts)
		: ts(ts)
	{
	}

	node_name_tester & test(const char * description,
							const char * mangled,
							const char * expectedResult)
	{
		ts << description;
		std::string result = tpie::pipelining::bits::extract_pipe_class_name(mangled);
		if (result != expectedResult) {
			tpie::log_debug()
				<< "Input:     " << mangled << '\n'
				<< "Got:       " << result << '\n'
				<< "Expected:  " << expectedResult << '\n';
			ts << tpie::result(false);
		} else {
			ts << tpie::result(true);
		}
		return *this;
	}

private:
	tpie::teststream & ts;
};

void gcc_test(tpie::teststream & ts) {
	node_name_tester(ts)
.test("multiply_t<TB::virtrecv<unsigned long> >", "10multiply_tIN4tpie10pipelining4bits8virtrecvImEEE", "multiply_t")
.test("multiply_t<multiply_t<TB::output_vector_t<unsigned long> > >", "10multiply_tIS_IN4tpie10pipelining4bits15output_vector_tImEEEE", "multiply_t")
.test("push_zero_t<TB::virtrecv<...> >", "11push_zero_tIN4tpie10pipelining4bits8virtrecvIRKNS0_5arrayImNS0_9allocatorImEEEEEEE", "push_zero_t")
.test("step_begin_type<TB::producer<unsigned long, unsigned long> >", "15step_begin_typeIN4tpie10pipelining13parallel_bits8producerImmEEE", "step_begin_type")
.test("push_in_end_type<TB::after<unsigned long> >", "16push_in_end_typeIN4tpie10pipelining13parallel_bits5afterImEEE", "push_in_end_type")
.test("sequence_verifier", "17sequence_verifier", "sequence_verifier")
.test("prepare_begin_type<prepare_middle_type<prepare_end_type> >", "18prepare_begin_typeI19prepare_middle_typeI16prepare_end_typeEE", "prepare_begin_type")
.test("sequence_generator<TB::producer<unsigned long, unsigned long> >", "18sequence_generatorIN4tpie10pipelining13parallel_bits8producerImmEEE", "sequence_generator")
.test("noop_initiator_type<TB::producer<unsigned long, unsigned long> >", "19noop_initiator_typeIN4tpie10pipelining13parallel_bits8producerImmEEE", "noop_initiator_type")
.test("buffering_accumulator_type<TB::after<unsigned long> >", "26buffering_accumulator_typeIN4tpie10pipelining13parallel_bits5afterImEEE", "buffering_accumulator_type")
.test("multiplicative_inverter_type<TB::after<unsigned long> >", "28multiplicative_inverter_typeIN4tpie10pipelining13parallel_bits5afterImEEE", "multiplicative_inverter_type")
.test("FF1<FF2<FF3> >", "3FF1I3FF2I3FF3EE", "FF1")
.test("FF2<FF3>", "3FF2I3FF3E", "FF2")
.test("FF3", "3FF3", "FF3")
.test("Summer", "6Summer", "Summer")
.test("Splitter<TB::after<unsigned long> >", "8SplitterIN4tpie10pipelining13parallel_bits5afterImEEE", "Splitter")
.test("Monotonic<TB::producer<unsigned long, unsigned long> >", "9MonotonicIN4tpie10pipelining13parallel_bits8producerImmEEE", "Monotonic")
.test("memtest_1<memtest_2>", "9memtest_1I9memtest_2E", "memtest_1")
.test("TB::identity_t<TB::dummydest_t<unsigned long> >", "N4tpie10pipelining4bits10identity_tINS1_11dummydest_tImEEEE", "identity_t")
.test("TB::bitbucket_t<...>", "N4tpie10pipelining4bits11bitbucket_tIRKNS_5arrayImNS_9allocatorImEEEEEE", "bitbucket_t")
.test("TB::dummydest_t<unsigned long>", "N4tpie10pipelining4bits11dummydest_tImEE", "dummydest_t")
.test("TB::pull_to_push<...>::pusher_t<...>", "N4tpie10pipelining4bits12pull_to_pushINS0_9factory_0INS1_15pull_identity_tEEEE8pusher_tINS1_8output_tImEEEE", "pull_to_push")
.test("TB::push_to_pull<...>::puller_t<...>", "N4tpie10pipelining4bits12push_to_pullINS0_9factory_0INS1_10identity_tEEEE8puller_tINS1_12pull_input_tImEEEE", "push_to_pull")
.test("TB::input_vector_t<TP::join<int>::sink_impl>", "N4tpie10pipelining4bits14input_vector_tINS0_4joinIiE9sink_implEEE", "input_vector_t")
.test("TB::extract_first_t<TB::output_vector_t<unsigned long> >", "N4tpie10pipelining4bits15extract_first_tINS1_15output_vector_tImEEEE", "extract_first_t")
.test("TB::output_vector_t<int>", "N4tpie10pipelining4bits15output_vector_tIiEE", "output_vector_t")
.test("TB::output_vector_t<unsigned long>", "N4tpie10pipelining4bits15output_vector_tImEE", "output_vector_t")
.test("TB::count_consecutive_t<...>", "N4tpie10pipelining4bits19count_consecutive_tINS1_15extract_first_tINS1_15output_vector_tImEEEEEE", "count_consecutive_t")
.test("TB::pull_input_iterator_t<...>", "N4tpie10pipelining4bits21pull_input_iterator_tIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEEEE", "pull_input_iterator_t")
.test("TB::push_input_iterator_t<...>::type<TB::push_output_iterator_t<...> >", "N4tpie10pipelining4bits21push_input_iterator_tIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEEE4typeINS1_22push_output_iterator_tIS9_vEEEE", "push_input_iterator_t")
.test("TB::pull_output_iterator_t<...>::type<TB::pull_input_iterator_t<...> >", "N4tpie10pipelining4bits22pull_output_iterator_tIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEEE4typeINS1_21pull_input_iterator_tIS9_EEEE", "pull_output_iterator_t")
.test("TB::push_output_iterator_t<...>", "N4tpie10pipelining4bits22push_output_iterator_tIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEEvEE", "push_output_iterator_t")
.test("TB::merge_t<...>::type<TB::output_t<unsigned long> >", "N4tpie10pipelining4bits7merge_tINS0_13termfactory_1INS1_12pull_input_tImEERNS_11file_streamImEEEEE4typeINS1_8output_tImEEEE", "merge_t")
.test("end_time::begin_type", "N8end_time10begin_typeE", "begin_type")
.test("end_time::end_type<end_time::begin_type>", "N8end_time8end_typeINS_10begin_typeEEE", "end_type")
		;
}

void msvc_test(tpie::teststream & ts) {
	node_name_tester(ts)
.test("multiply_t<TB::virtrecv<unsigned long> >", "struct multiply_t<class tpie::pipelining::bits::virtrecv<unsigned __int64> >", "multiply_t")
.test("multiply_t<multiply_t<TB::output_vector_t<unsigned long> > >", "struct multiply_t<struct multiply_t<class tpie::pipelining::bits::output_vector_t<unsigned __int64> > >", "multiply_t")
.test("push_zero_t<TB::virtrecv<...> >", "class push_zero_t<class tpie::pipelining::bits::virtrecv<class tpie::array<unsigned __int64,class tpie::allocator<unsigned __int64> > const & __ptr64> >", "push_zero_t")
.test("step_begin_type<TB::producer<unsigned long, unsigned long> >", "class step_begin_type<class tpie::pipelining::parallel_bits::producer<unsigned __int64,unsigned __int64> >", "step_begin_type")
.test("push_in_end_type<TB::after<unsigned long> >", "class push_in_end_type<class tpie::pipelining::parallel_bits::after<unsigned __int64> >", "push_in_end_type")
.test("sequence_verifier", "struct sequence_verifier", "sequence_verifier")
.test("prepare_begin_type<prepare_middle_type<prepare_end_type> >", "class prepare_begin_type<class prepare_middle_type<class prepare_end_type> >", "prepare_begin_type")
.test("sequence_generator<TB::producer<unsigned long, unsigned long> >", "struct sequence_generator<class tpie::pipelining::parallel_bits::producer<unsigned __int64,unsigned __int64> >", "sequence_generator")
.test("noop_initiator_type<TB::producer<unsigned long, unsigned long> >", "class noop_initiator_type<class tpie::pipelining::parallel_bits::producer<unsigned __int64,unsigned __int64> >", "noop_initiator_type")
.test("buffering_accumulator_type<TB::after<unsigned long> >", "class buffering_accumulator_type<class tpie::pipelining::parallel_bits::after<unsigned __int64> >", "buffering_accumulator_type")
.test("multiplicative_inverter_type<TB::after<unsigned long> >", "class multiplicative_inverter_type<class tpie::pipelining::parallel_bits::after<unsigned __int64> >", "multiplicative_inverter_type")
.test("FF1<FF2<FF3> >", "struct FF1<struct FF2<struct FF3> >", "FF1")
.test("FF2<FF3>", "struct FF2<struct FF3>", "FF2")
.test("FF3", "struct FF3", "FF3")
.test("Summer", "class Summer", "Summer")
.test("Splitter<TB::after<unsigned long> >", "class Splitter<class tpie::pipelining::parallel_bits::after<unsigned __int64> >", "Splitter")
.test("Monotonic<TB::producer<unsigned long, unsigned long> >", "class Monotonic<class tpie::pipelining::parallel_bits::producer<unsigned __int64,unsigned __int64> >", "Monotonic")
.test("memtest_1<memtest_2>", "class memtest_1<class memtest_2>", "memtest_1")
.test("TB::identity_t<TB::dummydest_t<unsigned long> >", "class tpie::pipelining::bits::identity_t<class tpie::pipelining::bits::dummydest_t<unsigned __int64> >", "identity_t")
.test("TB::bitbucket_t<...>", "class tpie::pipelining::bits::bitbucket_t<class tpie::array<unsigned __int64,class tpie::allocator<unsigned __int64> > const & __ptr64>", "bitbucket_t")
.test("TB::dummydest_t<unsigned long>", "class tpie::pipelining::bits::dummydest_t<unsigned __int64>", "dummydest_t")
.test("TB::pull_to_push<...>::pusher_t<...>", "class tpie::pipelining::bits::pull_to_push<class tpie::pipelining::factory_0<class tpie::pipelining::bits::pull_identity_t> >::pusher_t<class tpie::pipelining::bits::output_t<unsigned __int64> >", "pull_to_push")
.test("TB::push_to_pull<...>::puller_t<...>", "class tpie::pipelining::bits::push_to_pull<class tpie::pipelining::factory_0<class tpie::pipelining::bits::identity_t> >::puller_t<class tpie::pipelining::bits::pull_input_t<unsigned __int64> >", "push_to_pull")
.test("TB::input_vector_t<TP::join<int>::sink_impl>", "class tpie::pipelining::bits::input_vector_t<class tpie::pipelining::join<int>::sink_impl>", "input_vector_t")
.test("TB::extract_first_t<TB::output_vector_t<unsigned long> >", "class tpie::pipelining::bits::extract_first_t<class tpie::pipelining::bits::output_vector_t<unsigned __int64> >", "extract_first_t")
.test("TB::output_vector_t<int>", "class tpie::pipelining::bits::output_vector_t<int>", "output_vector_t")
.test("TB::output_vector_t<unsigned long>", "class tpie::pipelining::bits::output_vector_t<unsigned __int64>", "output_vector_t")
.test("TB::count_consecutive_t<...>", "class tpie::pipelining::bits::count_consecutive_t<class tpie::pipelining::bits::extract_first_t<class tpie::pipelining::bits::output_vector_t<unsigned __int64> > >", "count_consecutive_t")
.test("TB::pull_input_iterator_t<...>", "class tpie::pipelining::bits::pull_input_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > > >", "pull_input_iterator_t")
.test("TB::push_input_iterator_t<...>::type<TB::push_output_iterator_t<...> >", "class tpie::pipelining::bits::push_input_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > > >::type<class tpie::pipelining::bits::push_output_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > >,void> >", "push_input_iterator_t")
.test("TB::pull_output_iterator_t<...>::type<TB::pull_input_iterator_t<...> >", "class tpie::pipelining::bits::pull_output_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > > >::type<class tpie::pipelining::bits::pull_input_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > > > >", "pull_output_iterator_t")
.test("TB::push_output_iterator_t<...>", "class tpie::pipelining::bits::push_output_iterator_t<class std::_Vector_iterator<class std::_Vector_val<unsigned __int64,class std::allocator<unsigned __int64> > >,void>", "push_output_iterator_t")
.test("TB::merge_t<...>::type<TB::output_t<unsigned long> >", "class tpie::pipelining::bits::merge_t<class tpie::pipelining::termfactory_1<class tpie::pipelining::bits::pull_input_t<unsigned __int64>,class tpie::file_stream<unsigned __int64> & __ptr64> >::type<class tpie::pipelining::bits::output_t<unsigned __int64> >", "merge_t")
.test("end_time::begin_type", "class end_time::begin_type", "begin_type")
.test("end_time::end_type<end_time::begin_type>", "class end_time::end_type<class end_time::begin_type>", "end_type")
		;
}

int main(int argc, char ** argv) {
	return tpie::tests(argc, argv)
		.multi_test(gcc_test, "gcc")
		.multi_test(msvc_test, "msvc")
		;
}
