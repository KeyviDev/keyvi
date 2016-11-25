//
// Created by Narek Gharibyan on 11/21/16.
//

#ifndef KEYVI_FILE_H
#define KEYVI_FILE_H

#include <fstream>

#include "dictionary/fsa/internal/serialization_utils.h"

namespace keyvi {
namespace dictionary {
namespace util {

class KeyViFile {
    using ptree=boost::property_tree::ptree;
public:
    explicit KeyViFile(const std::string& filename)
        : fileStream_(filename, std::ios::binary)
    {
        using namespace ::boost;
        using namespace fsa::internal;

        if (!fileStream_.good()) {
            throw std::invalid_argument("file not found");
        }

        char magic[8];
        fileStream_.read(magic, sizeof(magic));
        // check magic
        if (std::strncmp(magic, "KEYVIFSA", 8)){
            throw std::invalid_argument("not a keyvi file");
        }

        automataProperties_ = SerializationUtils::ReadJsonRecord(fileStream_);
        persistenceOffset_ = fileStream_.tellg();

        // check for file truncation
        const ptree sparse_array_properties = SerializationUtils::ReadJsonRecord(fileStream_);
        const bool compact_size = lexical_cast<uint32_t> (sparse_array_properties.get<std::string>("version")) == 2;
        const size_t bucket_size = compact_size ? sizeof(uint16_t) : sizeof(uint32_t);
        const size_t array_size = lexical_cast<size_t>(sparse_array_properties.get<std::string>("size"));

        fileStream_.seekg((size_t)fileStream_.tellg() + array_size + bucket_size * array_size - 1);
        if (fileStream_.peek() == EOF) {
            throw std::invalid_argument("file is corrupt(truncated)");
        }

        fileStream_.get();
        valueStoreOffset_ = fileStream_.tellg();
    }

    ptree automataProperties() const {
        return automataProperties_;
    }

    std::istream& persistenceStream() {
        return fileStream_.seekg(persistenceOffset_);
    }

    std::istream& valueStoreStream() {
        return fileStream_.seekg(valueStoreOffset_);
    }

private:
    std::ifstream   fileStream_;
    ptree           automataProperties_;
    std::streampos  persistenceOffset_;
    std::streampos  valueStoreOffset_;
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */
#endif //KEYVI_FILE_H
