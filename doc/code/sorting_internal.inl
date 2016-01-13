#include <tpie/tpie.h> // for tpie::tpie_init
#include <random> // for std::mt19937
#include <vector> // for std::vector
#include <algorithm> // for std::generate
#include <tpie/parallel_sort.h> // for tpie::parallel_sort

int main() {
	tpie::tpie_init();
	std::mt19937 rng;
	std::vector<int> numbers(1 << 29);
	std::generate(numbers.begin(), numbers.end(), rng);
	tpie::parallel_sort(numbers.begin(), numbers.end(), std::less<int>());
	tpie::tpie_finish();
	return 0;
}
