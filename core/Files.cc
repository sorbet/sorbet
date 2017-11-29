#include "Context.h"
#include <vector>

using namespace std;

namespace ruby_typer {
namespace core {

vector<int> findLineBreaks(const std::string &s) {
    vector<int> res;
    int i = 0;
    for (auto c : s) {
        if (c == '\n') {
            res.push_back(i);
        }
        i++;
    }
    return res;
}

File::File(std::string &&path_, std::string &&source_, Type source_type)
    : source_type(source_type), path_(path_), source_(source_), line_breaks(findLineBreaks(this->source_)) {}

File &FileRef::file(GlobalState &gs) const {
    return gs.files[_id];
}

UTF8Desc File::path() {
    return this->path_;
}

UTF8Desc File::source() {
    return this->source_;
}
} // namespace core
} // namespace ruby_typer
