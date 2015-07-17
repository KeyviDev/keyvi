#include <type_traits>

int main() {
	return std::is_polymorphic<int>::value == 0;
}