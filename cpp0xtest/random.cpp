#include <random>

int main() {
	std::mt19937 mt_rand(42);
	return mt_rand();
}
