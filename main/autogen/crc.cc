#include "main/autogen/crc.h"
#include "CRC.h"

using namespace std;

namespace sorbet::autogen {

class CRCBuilderImpl : public CRCBuilder {
    const CRC::Table<std::uint32_t, 32> lookupTable;

public:
    CRCBuilderImpl() : lookupTable(CRC::CRC_32()) {}
    ~CRCBuilderImpl(){
        // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
    };

    u4 crc32(string_view data) const override {
        return CRC::Calculate(data.data(), data.size(), this->lookupTable);
    }
};

CRCBuilder::~CRCBuilder() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}

shared_ptr<CRCBuilder> CRCBuilder::create() {
    return make_shared<CRCBuilderImpl>();
}

} // namespace sorbet::autogen