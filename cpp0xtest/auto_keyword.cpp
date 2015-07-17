#include <vector>

int main() {
	std::vector<int> vec;
	for(auto i = 0; i < 100; ++i)
		vec.push_back(i);

	for(auto i = vec.begin(); i != vec.end(); ++i) {
		vec.push_back(0);
	}

	return 0;
}