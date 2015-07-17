#include <tpie/priority_queue.h> // for tpie::priority_queue
#include <tpie/tpie_assert.h> // for tp_assert macro
#include <tpie/tpie.h> // for tpie_init
void pq_test() {
	tpie::priority_queue<int> pq;
	for (int i = 10; i > 0; --i) {
		pq.push(i);
	}
	tp_assert(pq.size() == 10, "Incorrect size");
	for (int i = 1; i <= 10; ++i) {
		tp_assert(pq.top() == i, "Incorrect element");
		pq.pop();
	}
	tp_assert(pq.empty(), "Incorrect size");
}
int main() {
	tpie::tpie_init();
	tpie::get_memory_manager().set_limit(1<<30);
	pq_test();
	tpie::tpie_finish();
	return 0;
}
