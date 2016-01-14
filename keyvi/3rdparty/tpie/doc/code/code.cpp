#include <tpie/dummy_progress.h>
#include <tpie/file_stream.h>
#include <tpie/fractional_progress.h>
#include <tpie/parallel_sort.h>
#include <tpie/priority_queue.h>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/serialization2.h>
#include <tpie/serialization_stream.h>
#include <tpie/serialization_sorter.h>
#include <tpie/sort.h>
#include <tpie/stack.h>
#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <algorithm>
#include <random>
#include <string>
#include <vector>

namespace _a {
#include "file_stream.inl"
}

namespace _b {
using namespace tpie;
void go() {
}
void fractiondb() {
#include "fractiondb.inl"
}
}

namespace _c {
void memory() {
#include "memory.inl"
}
}

namespace _d {
void memory() {
	const int megabytes = 1;
#include "memory2.inl"
}
}

namespace _e {
#include "priority_queue.inl"
}

namespace _f {
const size_t s = 1;
const size_t y = 1;
#include "progress1.inl"
}

namespace _g {
using namespace tpie;
const int elements = 1;
inline void write_number_stream(fractional_subindicator &) {}
inline void sort_number_stream(fractional_subindicator &) {}
void progress() {
#include "progress2.inl"
}
}

namespace _h {
#include "sorting_external.inl"
}

namespace _i {
#include "sorting_internal.inl"
}

namespace _j {
#include "progress3.inl"
}

namespace _k {
#include "serialization.inl"
}

int main() {
	return 0;
}
