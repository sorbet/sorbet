#include "Context.h"
#include <regex>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace core {

vector<int> findLineBreaks(const std::string &s) {
    vector<int> res;
    int i = -1;
    res.push_back(-1);
    for (auto c : s) {
        i++;
        if (c == '\n') {
            res.push_back(i);
        }
    }
    res.push_back(i);
    return res;
}
bool fileIsTyped(absl::string_view source) {
    static regex sigil("^\\s*#\\s*@typed\\s*$");
    size_t off = 0;
    // std::regex appears to be ludicrously slow, to the point where running
    // this regex is as slow as running the entirety of the remainder of our
    // pipeline.
    //
    // Help it out by manually scanning for the `@typed` literal, and then
    // running the regex only over the single line.
    while (true) {
        off = source.find("@typed", off);
        if (off == absl::string_view::npos) {
            return false;
        }

        size_t line_start = source.rfind('\n', off);
        if (line_start == absl::string_view::npos) {
            line_start = 0;
        }
        size_t line_end = source.find('\n', off);
        if (line_end == absl::string_view::npos) {
            line_end = source.size();
        }
        if (regex_search(source.data() + line_start, source.data() + line_end, sigil)) {
            return true;
        }
        off = line_end;
    }
}

File::File(std::string &&path_, std::string &&source_, Type source_type)
    : source_type(source_type), path_(path_), source_(source_),
      hashKey_(this->path_ + "-" + to_string(_hash(this->source_))), hasTypedSigil(fileIsTyped(this->source())),
      isTyped(hasTypedSigil) {}

FileRef::FileRef(unsigned int id) : _id(id) {}

const File &FileRef::data(const GlobalState &gs) const {
    ENFORCE(_id < gs.filesUsed());
    ENFORCE(gs.files[_id]->source_type != File::TombStone);
    return *(gs.files[_id]);
}

File &FileRef::data(GlobalState &gs) const {
    ENFORCE(_id < gs.filesUsed());
    ENFORCE(gs.files[_id]->source_type != File::TombStone);
    return *(gs.files[_id]);
}

absl::string_view File::path() const {
    return this->path_;
}

absl::string_view File::source() const {
    return this->source_;
}

absl::string_view File::hashKey() const {
    return this->hashKey_;
}

bool File::hadErrors() const {
    return hadErrors_;
}

bool File::isPayload() const {
    return source_type == Type::PayloadGeneration || source_type == Type::Payload;
}

std::vector<int> &File::line_breaks() const {
    auto ptr = std::atomic_load(&line_breaks_);
    if (ptr) {
        return *ptr;
    } else {
        auto my = make_shared<vector<int>>(findLineBreaks(this->source_));
        atomic_compare_exchange_weak(&line_breaks_, &ptr, my);
        return line_breaks();
    }
}

int File::lineCount() const {
    return line_breaks().size() - 1;
}

} // namespace core
} // namespace ruby_typer
