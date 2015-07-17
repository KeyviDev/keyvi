#include <tuple>

template<typename... TS> 
class abe: public TS... {
 public:
};

class bar {};

int main() {
  abe<abe<abe<> >, bar>();
  std::tuple<int, int> a = std::make_tuple(1, 3);
}
