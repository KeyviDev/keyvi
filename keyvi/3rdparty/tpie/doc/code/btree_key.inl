typedef std::pair<int, int> point;

struct point_key_func {
	typedef int value_type;

	int operator()(const point & t) const {
		return t.first;
	}
};
