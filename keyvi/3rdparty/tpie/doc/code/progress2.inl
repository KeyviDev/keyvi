progress_indicator_arrow pi("Hello world", elements);
fractional_progress fp(&pi);

fractional_subindicator progress_writer(fp, "Writer", TPIE_FSI, elements, "Writer");
fractional_subindicator progress_sort(fp, "Sort", TPIE_FSI, elements, "Sort");

fp.init();
write_number_stream(progress_writer);
sort_number_stream(progress_sort);
fp.done();
