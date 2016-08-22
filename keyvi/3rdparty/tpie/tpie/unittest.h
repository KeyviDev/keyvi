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

#ifndef __TPIE_UNITTEST_H__
#define __TPIE_UNITTEST_H__

#include <iostream>
#include <tpie/types.h>
#include <tpie/logstream.h>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <chrono>
#include <stack>

namespace tpie {

typedef std::chrono::high_resolution_clock test_clock;
typedef std::chrono::time_point<test_clock> test_time;
inline test_time test_now() {return test_clock::now();}
inline double test_millisecs(const test_time & from, const test_time & to) {
	return std::chrono::duration_cast<std::chrono::duration<double, std::milli> >(to-from).count();
}
	
inline double test_secs(const test_time & from, const test_time & to) {
	return std::chrono::duration_cast<std::chrono::duration<double>>(to-from).count();
}			
	
class teststream_buf: public std::basic_streambuf<char, std::char_traits<char> > {
private:
	const static size_t line_size = 2048;
	char m_line[line_size];
	bool m_new_line;
public:
	teststream_buf();
	virtual int overflow(int c = traits_type::eof());
	void stateAlign();
	virtual int sync();
};

class teststream: public std::ostream  {
private:
	teststream_buf m_buff;
	size_t failed;
	size_t total;
	test_time time;
	bool do_time;
public:
	teststream(bool do_time);
	bool success();
	friend void result_manip(teststream & s, bool success);
};

template <class TP>
class testmanip {
	void (*_f)(teststream&, TP);
	TP _a;
public:
	inline testmanip(void (*f)(teststream&, TP), TP a) : _f(f), _a(a) {}
	inline friend std::ostream& operator<<(std::ostream& o, const testmanip<TP>& m) {
		(*m._f)(static_cast<teststream&>(o), m._a);
		return o;
	}
};

testmanip<bool> result(bool success);
testmanip<bool> success();
testmanip<bool> failure();

#define TEST_ENSURE(cond, message) {									\
		if (! (cond) ) {												\
			tpie::log_error() << message								\
							  << " (line " << __LINE__ << ")" << std::endl;	\
			return false;												\
		}																\
	}																	
	
#define TEST_ENSURE_EQUALITY(v1, v2, message) {							\
		if ( !((v1) == (v2)) ) {										\
			tpie::log_error() << message								\
							  << " (" << v1 << " != " << v2				\
							  << " line " << __LINE__ << ")" << std::endl; \
			return false; 												\
		}																\
	}																	

#define TEST_FAIL(message) {											\
		tpie::log_error() << message									\
						  << " (line " << __LINE__ << ")" << std::endl;	\
		return false;													\
	}																		
	

class tests;

namespace bits {
	class test_runner {
		tests * t;
		bool result;
		test_time m_time;
	public:
		test_runner(tests * t, const std::string & name);

		void set_result(bool result);
		void exception();

		~test_runner();
	};
} // namespace bits

class tests {
public:
	static const size_t lineLength = 79;

	tests(int argc, char ** argv, memory_size_type memory_limit=50);
	virtual ~tests();

	template <typename T>
	tests & setup(T t);

	template <typename T>
	tests & finish(T t);
	
	template <typename T>
	tests & test(T fct, const std::string & name);
	
	template <typename T, typename T1>
	tests & test(T fct, const std::string & name, 
				 const std::string & p1_name, T1 p1_default);

	template <typename T, typename T1, typename T2>
	tests & test(T fct, const std::string & name, 
				 const std::string & p1_name, T1 p1_default, 
				 const std::string & p2_name, T2 p2_default);

	template <typename T, typename T1, typename T2, typename T3>
	tests & test(T fct, const std::string & name, 
				 const std::string & p1_name, T1 p1_default, 
				 const std::string & p2_name, T2 p2_default,
				 const std::string & p3_name, T3 p3_default);

	template <typename T, typename T1, typename T2, typename T3, typename T4>
	tests & test(T fct, const std::string & name, 
				 const std::string & p1_name, T1 p1_default, 
				 const std::string & p2_name, T2 p2_default,
				 const std::string & p3_name, T3 p3_default,
				 const std::string & p4_name, T4 p4_default);

	template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	tests & test(T fct, const std::string & name, 
				 const std::string & p1_name, T1 p1_default, 
				 const std::string & p2_name, T2 p2_default,
				 const std::string & p3_name, T3 p3_default,
				 const std::string & p4_name, T4 p4_default,
				 const std::string & p5_name, T5 p5_default);
	
	template <typename T>
	tests & multi_test(T fct, const std::string & name);

	template <typename T, typename T1>
	tests & multi_test(T fct, const std::string & name, const std::string & p1_name, T1 p1_default);
	
	template <typename T, typename T1, typename T2>
	tests & multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default);

	template <typename T, typename T1, typename T2, typename T3>
	tests & multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default);

	template <typename T, typename T1, typename T2, typename T3, typename T4>
	tests & multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default);

	template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
	tests & multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default,
			const std::string & p5_name, T5 p5_default);

	template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	tests & multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default,
			const std::string & p5_name, T5 p5_default,
			const std::string & p6_name, T6 p6_default);



	operator int();
protected:
	virtual void build_information(std::ostream & o);
private:
	void show_usage(std::ostream & o);

	struct func {
		virtual void operator()() = 0;
		virtual ~func() {}
	};

	template <typename T>
	struct func_impl: func {
		T t;
		func_impl(T t): t(t) {}
		virtual void operator()() {t();}
	};


	struct test_log_target: public log_target {
		test_log_target(std::ostream & os = std::cout)
			: os(os)
			, threshold(LOG_FATAL)
			, bufferThreshold(LOG_FATAL)
			, m_onNameLine(false)
			, m_onBOL(true)
		{
		}

		~test_log_target() {
			if (!m_onBOL) os << std::endl;
		}

		void set_test(const std::string & name) {
			buffer.str("");
			m_onNameLine = false;
			m_name = name;
			set_status("    ");
		}

		void set_status(const std::string & status) {
			if (m_onNameLine) {
				os << '\r';
			} else if (!m_onBOL) {
				os << '\n';
			}

			os << m_name << ' ';
			const size_t maxNameSize = lineLength
				- 2 // spaces before and after dots
				- 6; // status message: "[STAT]"
			if (maxNameSize > m_name.size()) os << std::string(maxNameSize-m_name.size(), '.');
			os << ' ';

			os << '[' << status << ']' <<  std::flush;
			m_onNameLine = true;
			m_onBOL = false;
		}

		void write_time(size_t n) {
			os << " " << n << " ms" << std::flush;
		}
			
		void log(log_level level, const char * buf, size_t n) {
			if (!n) return;
			if (level <= bufferThreshold) {
				std::string prefix = LOG_DEBUG > bufferThreshold ? "" : (build_prefix(groups.size()) + " ");
				std::string msg =  prefix + std::string(buf, n);
				buffer << msg;
			}
			if (level > threshold) return;
			if (m_onNameLine) os << '\n';

			std::string prefix = LOG_DEBUG > threshold ? "" : (build_prefix(groups.size()) + " ");
			std::string msg = prefix + std::string(buf, n);
	
			os << msg;
			m_onNameLine = false;
			m_onBOL = msg[msg.size()-1] == '\n' || msg[msg.size()-1] == '\r';
			os << std::flush;
		}

		void set_threshold(log_level level) {
			threshold = level;
		}

		void set_buffer_threshold(log_level level) {
			bufferThreshold = level;
		}

		void begin_group(const std::string & name) {
			groups.push(name);
			std::string msg = build_prefix(groups.size()-1) + "> " + "Entering " + name + '\n';

			if (LOG_DEBUG <= bufferThreshold) buffer << msg;
			if (LOG_DEBUG > threshold) return;
			if (m_onNameLine) os << '\n';
			
			os << msg << std::flush;
			m_onNameLine = false;
			m_onBOL = true;
		}

		void end_group() {
			std::string msg = build_prefix(groups.size()-1) + "x " + "Leaving " + groups.top() + '\n';
			groups.pop();

			if (LOG_DEBUG <= bufferThreshold) buffer << msg;
			if (LOG_DEBUG > threshold) return;
			if (m_onNameLine) os << '\n';
			
			os << msg << std::flush;
			m_onNameLine = false;
			m_onBOL = true;
		}

		std::stringstream buffer;
	private:
		std::string build_prefix(size_t size) {
			return std::string(size, '|');
		}

		std::stack<std::string> groups;
		std::ostream & os;
		log_level threshold;
		log_level bufferThreshold;
		bool m_onNameLine;
		std::string m_name;
		bool m_onBOL;
	};
	

	void start_test(const std::string & name);
	void end_test(bool result, size_t time);

	template <typename T>
	T get_arg(const std::string & name, T def) const;

	template <typename T>
	std::string arg_str(const std::string & name, T def) const;

	bool bad, usage, version, do_time;
	size_t tests_runned;
	std::string exe_name;
	std::string test_name;
	bool testAll;
	std::map<std::string, std::string> args;
	memory_size_type memory_limit;
	std::string current_name;
	std::vector<func *> setups;
	std::vector<func *> finishs;
	test_log_target log;
	std::vector<std::string> m_tests;

	friend class bits::test_runner;
};

template <typename src, typename dst>
struct magic_cast_help {
	dst operator()(const src & s) {
		return boost::lexical_cast<dst>(s);
	}
};

template <>
struct magic_cast_help<bool, std::string> {
	std::string operator()(bool b) {
		return b?"true":"false";
	}
};

template <>
struct magic_cast_help<std::string, bool> {
	bool operator()(std::string v) {
		std::transform(v.begin(), v.end(), v.begin(), tolower);
		return v == "true" || v == "1" || v == "on" || v == "yes";
	}
};

template <typename dst, typename src>
dst magic_cast(const src & s) {
	return magic_cast_help<src, dst>()(s);
}
	
template <typename T>
struct get_arg_help {
	T operator()(const std::map<std::string, std::string> & args, const std::string & name, T def) {
		T arg=def;
		try {
			std::map<std::string, std::string>::const_iterator i=args.find(name);
			if (i != args.end()) arg=magic_cast<T>(i->second);
		} catch (std::bad_cast) {
			std::cerr << "The argument " << name << " has the wrong type" << std::endl;
		}
		return arg;
	}
};

template <>
struct get_arg_help<bool> {
	bool operator()(const std::map<std::string, std::string> & args, const std::string & name, bool) {
		return args.count(name) != 0;
	}
};	

template <typename T>
T tests::get_arg(const std::string & name, T def) const {
	return get_arg_help<T>()(args, name, def);
}
	
template <typename T>
std::string tests::arg_str(const std::string & name, T def) const {
	std::string dashes((name.size() == 1) ? 1 : 2, '-');
	return std::string(" [")+dashes+name+" ARG (= "+magic_cast<std::string>(def)+")]";
}

template <typename T>
tests & tests::test(T fct, const std::string & name) {
	m_tests.push_back(name);
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1>
tests & tests::test(T fct, const std::string & name, const std::string & p1_name, T1 p1_default) {
	m_tests.push_back(name 
					  + arg_str(p1_name, p1_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct(get_arg(p1_name, p1_default)));
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}


template <typename T, typename T1, typename T2>
tests & tests::test(T fct, const std::string & name, const std::string & p1_name, T1 p1_default, const std::string & p2_name, T2 p2_default) {
	m_tests.push_back(name 
					  + arg_str(p1_name, p1_default) 
					  + arg_str(p2_name, p2_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct(get_arg(p1_name, p1_default),
						 get_arg(p2_name, p2_default)));
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}
	
template <typename T, typename T1, typename T2, typename T3>
tests & tests::test(T fct, const std::string & name, 
					const std::string & p1_name, T1 p1_default, 
					const std::string & p2_name, T2 p2_default,
					const std::string & p3_name, T3 p3_default) {
	m_tests.push_back(name 
					  + arg_str(p1_name, p1_default) 
					  + arg_str(p2_name, p2_default)
					  + arg_str(p3_name, p3_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct(get_arg(p1_name, p1_default),
						 get_arg(p2_name, p2_default),
						 get_arg(p3_name, p3_default)));
		} catch (...) {
			t.exception();
		}
	}
	return *this;

}

template <typename T, typename T1, typename T2, typename T3, typename T4>
tests & tests::test(T fct, const std::string & name, 
					const std::string & p1_name, T1 p1_default, 
					const std::string & p2_name, T2 p2_default,
					const std::string & p3_name, T3 p3_default,
					const std::string & p4_name, T4 p4_default) {
	m_tests.push_back(name 
					  + arg_str(p1_name, p1_default) 
					  + arg_str(p2_name, p2_default)
					  + arg_str(p3_name, p3_default)
					  + arg_str(p4_name, p4_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct(get_arg(p1_name, p1_default),
						 get_arg(p2_name, p2_default),
						 get_arg(p3_name, p3_default),
						 get_arg(p4_name, p4_default)));
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
tests & tests::test(T fct, const std::string & name, 
					const std::string & p1_name, T1 p1_default, 
					const std::string & p2_name, T2 p2_default,
					const std::string & p3_name, T3 p3_default,
					const std::string & p4_name, T4 p4_default,
					const std::string & p5_name, T5 p5_default) {
	m_tests.push_back(name 
					  + arg_str(p1_name, p1_default) 
					  + arg_str(p2_name, p2_default)
					  + arg_str(p3_name, p3_default)
					  + arg_str(p4_name, p4_default)
					  + arg_str(p5_name, p5_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			t.set_result(fct(get_arg(p1_name, p1_default),
						 get_arg(p2_name, p2_default),
						 get_arg(p3_name, p3_default),
						 get_arg(p4_name, p4_default),
						 get_arg(p5_name, p5_default)));
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}


template <typename T>
tests & tests::multi_test(T fct, const std::string & name) {
	m_tests.push_back(name);
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts);
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1>
tests & tests::multi_test(T fct, const std::string & name, const std::string & p1_name, T1  p1_default) {
	m_tests.push_back(name+ arg_str(p1_name, p1_default));
	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts, get_arg(p1_name, p1_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2>
tests & tests::multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default) {
	m_tests.push_back(name+ 
		arg_str(p1_name, p1_default) +
		arg_str(p2_name, p2_default));

	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts,
				get_arg(p1_name, p1_default),
				get_arg(p2_name, p2_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2, typename T3>
tests & tests::multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default) {
	m_tests.push_back(name+ 
		arg_str(p1_name, p1_default) +
		arg_str(p2_name, p2_default) +
		arg_str(p3_name, p3_default));

	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts,
				get_arg(p1_name, p1_default),
				get_arg(p2_name, p2_default),
				get_arg(p3_name, p3_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
tests & tests::multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default) {
	m_tests.push_back(name+
		arg_str(p1_name, p1_default) +
		arg_str(p2_name, p2_default) +
		arg_str(p3_name, p3_default) +
		arg_str(p4_name, p4_default));

	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts,
				get_arg(p1_name, p1_default),
				get_arg(p2_name, p2_default),
				get_arg(p3_name, p3_default),
				get_arg(p4_name, p4_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
tests & tests::multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default,
			const std::string & p5_name, T5 p5_default) {
	m_tests.push_back(name+
		arg_str(p1_name, p1_default) +
		arg_str(p2_name, p2_default) +
		arg_str(p3_name, p3_default) +
		arg_str(p4_name, p4_default) +
		arg_str(p5_name, p5_default));

	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts,
				get_arg(p1_name, p1_default),
				get_arg(p2_name, p2_default),
				get_arg(p3_name, p3_default),
				get_arg(p4_name, p4_default),
				get_arg(p5_name, p5_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
tests & tests::multi_test(T fct, const std::string & name,
			const std::string & p1_name, T1 p1_default,
			const std::string & p2_name, T2 p2_default,
			const std::string & p3_name, T3 p3_default,
			const std::string & p4_name, T4 p4_default,
			const std::string & p5_name, T5 p5_default,
			const std::string & p6_name, T6 p6_default) {
	m_tests.push_back(name+
		arg_str(p1_name, p1_default) +
		arg_str(p2_name, p2_default) +
		arg_str(p3_name, p3_default) +
		arg_str(p4_name, p4_default) +
		arg_str(p5_name, p5_default) +
		arg_str(p6_name, p6_default));

	if (testAll || name == test_name) {
		bits::test_runner t(this, name);
		try {
			teststream ts(do_time);
			fct(ts,
				get_arg(p1_name, p1_default),
				get_arg(p2_name, p2_default),
				get_arg(p3_name, p3_default),
				get_arg(p4_name, p4_default),
				get_arg(p5_name, p5_default),
				get_arg(p6_name, p6_default));
			t.set_result(ts.success());
		} catch (...) {
			t.exception();
		}
	}
	return *this;
}

template <typename T>
tests & tests::setup(T t) {
	setups.push_back(new func_impl<T>(t));
	return *this;
}

template <typename T>
tests & tests::finish(T t) {
	finishs.push_back(new func_impl<T>(t));
	return *this;
}

} //namespace tpie

#endif // __TPIE_UNITTEST_H__
