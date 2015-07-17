#include <tpie/dummy_progress.h>
#include <tpie/types.h>
#include <tpie/progress_indicator_arrow.h>

tpie::file_stream<int> fs;
tpie::uint64_t item_count;

template <bool use_progress>
void scanner(typename tpie::progress_types<use_progress>::base * pi) {
	pi->init(item_count);
	for (tpie::uint64_t i = 0; i < item_count; ++i) {
		fs.read();
		pi->step();
	}
	pi->done();
}

void scanner_optional_progress(bool use_progress) {
	if (use_progress) {
		tpie::progress_indicator_arrow pi("Hello world", item_count);
		scanner<true>(&pi);
	} else {
		tpie::dummy_progress_indicator pi;
		scanner<false>(&pi);
	}
}
