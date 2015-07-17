// Checking for decltype support is not so easy,
// since MSVC 2010 has a broken implementation.
// The issue might be related to this bug report which is marked as fixed:
// https://connect.microsoft.com/VisualStudio/feedback/details/678194/
// We need decltype support for the push_type/pull_type helpers in pipelining.

template <typename T>
struct traits {
	static const bool is_specialized = false;
};

template <typename ClassType, typename ArgType>
struct traits< void(ClassType::*)(ArgType) > {
	typedef ArgType t;
	static const bool is_specialized = true;
};

template <typename T>
struct help {
	typedef decltype(&T::bar) t1;
	typedef traits<t1> t2;

	// The following line fails to parse on MSVC.
	typedef typename t2::t type;

	static const bool is_specialized = t2::is_specialized;
};

struct foo {
	void bar(int) {}
};

int main() {
	static_assert(help<foo>::is_specialized, "method pointer was not matched");
	return 0;
}
