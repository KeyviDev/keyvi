#include <fstream>
#include <iostream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <json/json.h>

#include <msgpack.hpp>

#include "msgpack/type/rapidjson.hpp"
#include "msgpack/type/jsoncpp.hpp"

using namespace rapidjson;
using namespace msgpack;


struct FileFormat {
    enum Format {
        INVALID,
        JSON,
        MSGPACK,
    };
    std::string filename;
    Format format;
    static Format formatForExtension(const std::string& extension)
    {
        if (extension == ".json")
            return JSON;
        if (extension == ".mpack")
            return MSGPACK;
        return INVALID;
    }
};

struct Opt {
    FileFormat src;
    FileFormat dest;
    std::string executable;
    bool help;

    bool parse(int argc, char* argv[])
    {
        executable = program(argv[0]);
        std::cout << executable << std::endl;

        if (argc != 4)
        {
            usage();
            return false;
        }


        for (int i = 1; i < argc; i++)
        {
            std::string arg(argv[i]);
            if (arg == "-o")
            {
                if (i == argc - 1) // last arg should not be "-o"
                {
                    usage();
                    return false;
                }

                dest.filename = argv[++i];
            }
            else
                src.filename = arg;
        }

        if (src.filename.empty() && dest.filename.empty())
        {
            usage();
            return false;
        }
        if (!getFormat(dest.filename, &dest.format))
            return false;
        if (!getFormat(src.filename, &src.format))
            return false;

        if (src.format == dest.format)
        {
            std::cerr << "Same format, nothing to do: " << src.filename << ", " << dest.filename << std::endl;
            return false;
        }


        return true;
    }

private:
    void usage() const
    {
        std::cerr << "Usage " << executable << " -o <outfile> <inputfile>" << std::endl;;
    }
    bool getFormat(const std::string& filename, FileFormat::Format* f)
    {
        *f = FileFormat::formatForExtension(extension(filename));
        bool rv = (*f != FileFormat::INVALID);
        if (!rv)
            std::cerr << "Unsupported extension:" << filename << std::endl;
        return rv;

    }
    static std::string extension(const std::string& filename)
    {
        size_t start = filename.find_last_of(".");
        if (start == std::string::npos)
            return "";
        size_t check = filename.find_last_of("/\\");
        if (check != std::string::npos && start < check)
            return "";
        return filename.substr(start, filename.size() - start);
    }
    static std::string program(const std::string& argv0)
    {
        size_t start = argv0.find_last_of("/\\");
        start = (start == std::string::npos) ? 0 : start + 1;
        size_t ends = argv0.rfind(".");
        if (ends == std::string::npos || ends < start)
            ends = argv0.size();

        return argv0.substr(start, ends - start);
    }

};

bool read_file_contents(const std::string& filename, std::string* contents)
{
	std::ifstream in(filename.data(), std::ios::in | std::ios::binary);
	if (in.bad())
		return false;

	in.seekg(0, std::ios::end);
	contents->resize(static_cast<size_t>(in.tellg()));
	in.seekg(0, std::ios::beg);
	in.read(&(*contents)[0], contents->size());
	in.close();
	return true;
}
bool write_file_contents(const std::string& filename, const std::string& contents)
{
	std::ofstream of(filename.data(), std::ios::out | std::ios::binary);
	if (of.bad())
		return false;
	of.write(contents.data(), contents.size());
	return true;
}


struct Msgpack {
    struct Document {
        std::string buffer;
        msgpack::unpacked unpacked;
    };
    typedef Document document_type;;
    static bool load(Document& doc, const std::string& filename)
    {
        if (!read_file_contents(filename, &doc.buffer))
            return false;
        msgpack::unpack(&doc.unpacked, doc.buffer.data(), doc.buffer.size());
        return true;
    }
    static bool save(const Json::Value& doc, const std::string& filename)
    {
        msgpack::sbuffer sbuf;  // simple buffer
        msgpack::pack(&sbuf, doc);

        return write_file_contents(filename, std::string(sbuf.data(), sbuf.size()));
    }
    static bool save(const rapidjson::Document& doc, const std::string& filename)
    {
        msgpack::sbuffer sbuf;  // simple buffer
        msgpack::pack(&sbuf, doc);

        return write_file_contents(filename, std::string(sbuf.data(), sbuf.size()));
    }
};

struct Jsoncpp {
    typedef Json::Value document_type;;

    static bool load(Json::Value& doc, const std::string& filename)
    {
        Json::Reader reader;
        std::ifstream in(filename.data(), std::ios::in | std::ios::binary);
        reader.parse(in, doc);
        return true;
    }
    static bool save(const Msgpack::Document& sdoc, const std::string& filename)
    {
        Json::Value doc;

        sdoc.unpacked.get().convert(&doc);
        std::ofstream of(filename.data(), std::ios::out | std::ios::binary);
        of << doc;
        return true;
    }
};

struct RapidJSON {
    typedef rapidjson::Document document_type;;
    static bool load(rapidjson::Document& doc, const std::string& filename)
    {
        std::string contents;
        if (!read_file_contents(filename, &contents))
            return false;
        doc.Parse(contents.data());
        return true;
    }
    static bool save(const Msgpack::Document& sdoc, const std::string& filename)
    {
        rapidjson::Document doc;

        sdoc.unpacked.get().convert(&doc);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        return write_file_contents(filename, std::string(buffer.GetString(), buffer.GetSize()));
    }
};

template <typename Src, typename Dest>
bool convert(const std::string& sf, const std::string& df)
{
    typename Src::document_type doc;
    if (!Src::load(doc, sf))
        return false;
    return Dest::save(doc, df);
}




int main(int argc, char* argv[])
{
    Opt opt;
    if (!opt.parse(argc, argv))
        return EXIT_FAILURE;
    if (opt.src.format == FileFormat::JSON && opt.dest.format == FileFormat::MSGPACK)
        return convert<Jsoncpp, Msgpack>(opt.src.filename, opt.dest.filename) ? EXIT_SUCCESS : EXIT_FAILURE;
    if (opt.src.format == FileFormat::MSGPACK && opt.dest.format == FileFormat::JSON)
        return convert<Msgpack, Jsoncpp>(opt.src.filename, opt.dest.filename) ? EXIT_SUCCESS : EXIT_FAILURE;

	return 0;
}
