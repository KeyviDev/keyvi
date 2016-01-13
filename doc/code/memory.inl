int* my_ints = tpie::tpie_new_array<int>(1024);
std::fill(my_ints+0, my_ints+1024, 0);
tpie::tpie_delete_array(my_ints, 1024);

tpie::unique_ptr<tpie::stack<int> > mystack(tpie::tpie_new<tpie::stack<int> >("stack.tpie"));
mystack->push(42);
// when mystack goes out of scope, the stack object is deleted with tpie_delete.
