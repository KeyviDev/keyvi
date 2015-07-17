void populate_stream(tpie::file_stream<size_t> & writer) {
	// The parameter to init tells the PI how many times we will call step().
	tpie::progress_indicator_arrow progress_writer("Populate stream", s);
	progress_writer.init(s);
	for (size_t i = 0; i < s; ++i) {
		// Write a single item
		writer.write((i * y) % s);
		progress_writer.step();
	}
	progress_writer.done();
}
