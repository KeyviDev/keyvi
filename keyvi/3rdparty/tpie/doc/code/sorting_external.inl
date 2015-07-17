#include <tpie/tpie.h> // for tpie::tpie_init
#include <tpie/sort.h> // for tpie::sort

int main() {
	tpie::tpie_init();

	//create a stream with 1000 ints
	tpie::file_stream<int> mystream;

	for (int i = 0; i < 1000; ++i) {
	    mystream.write(i);
	}

	//sort the stream using the "<" operator
	tpie::sort(mystream,mystream);

	tpie::tpie_finish();

	return 0;
}


