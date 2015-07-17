void write_lines(std::istream & is, std::string filename) {
	std::string line;
	tpie::serialization_writer wr;
	wr.open(filename);
	while (std::getline(is, line)) {
		wr.serialize(line);
	}
	wr.close();
}

void reverse_lines(std::string filename) {
	tpie::temp_file f;
	{
		tpie::serialization_reader rd;
		rd.open(filename);
		tpie::serialization_reverse_writer wr;
		wr.open(f);
		while (rd.can_read()) {
			std::string line;
			rd.unserialize(line);
			wr.serialize(line);
		}
		wr.close();
		rd.close();
	}
	{
		tpie::serialization_reverse_reader rd;
		rd.open(f);
		tpie::serialization_writer wr;
		wr.open(filename);
		while (rd.can_read()) {
			std::string line;
			rd.unserialize(line);
			wr.serialize(line);
		}
		wr.close();
		rd.close();
	}
}

void read_lines(std::ostream & os, std::string filename) {
	tpie::serialization_reader rd;
	rd.open(filename);
	while (rd.can_read()) {
		std::string line;
		rd.unserialize(line);
		os << line << '\n';
	}
	rd.close();
}

void sort_lines(std::string filename) {
	tpie::serialization_sorter<std::string> sorter;
	sorter.set_available_memory(50*1024*1024);
	sorter.begin();
	{
		tpie::serialization_reader rd;
		rd.open(filename);
		while (rd.can_read()) {
			std::string line;
			rd.unserialize(line);
			sorter.push(line);
		}
		rd.close();
	}
	sorter.end();
	sorter.merge_runs();
	{
		tpie::serialization_writer wr;
		wr.open(filename);
		while (sorter.can_pull()) {
			wr.serialize(sorter.pull());
		}
		wr.close();
	}
}
