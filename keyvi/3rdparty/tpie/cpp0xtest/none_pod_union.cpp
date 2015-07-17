#include <string>

struct monkey {
	union {
		std::string a;
		int b;
	};
  
	monkey() {}
	~monkey() {}
};

int main() {
	monkey k;
	k.b=0;
	return k.b;
}
