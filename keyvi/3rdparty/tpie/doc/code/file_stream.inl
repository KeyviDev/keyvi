#include <tpie/file_stream.h>
#include <string>

using namespace tpie;
using namespace std;

struct segment_t {
	double x1, y1, x2, y2;
	float z1, z2;
};

void copystream(const string & infile, const string & outfile) {
	file_stream<segment_t> in;
	file_stream<segment_t> out;
	in.open(infile);
	out.open(outfile);
	while (in.can_read()) {
		segment_t item = in.read();
		out.write(item);
	}
}
