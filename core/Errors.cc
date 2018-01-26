#include "core/errors/errors.h"
#include "Context.h"
#include "spdlog/fmt/ostr.h"
#include <algorithm>

namespace ruby_typer {
namespace core {

using namespace std;

BasicError::BasicError(Loc loc, ErrorClass what, std::string formatted)
    : loc(loc), what(what), formatted(formatted), isCritical(false) {
    ENFORCE(formatted.find("\n") == std::string::npos, formatted, " has a newline in it");
}

string BasicError::filePosToString(const GlobalState &gs, Loc loc) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:";
    } else {
        auto pos = loc.position(gs);
        buf << loc.file.file(gs).path() << ":";
        buf << pos.first.line;
        if (pos.second.line != pos.first.line) {
            buf << "-";
            buf << pos.second.line;
        }
    }
    return buf.str();
}

string BasicError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << filePosToString(gs, loc) << " " << formatted << " [" << what.code << "]" << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs);
    }
    return buf.str();
}

string ErrorLine::toString(const GlobalState &gs) {
    stringstream buf;
    string indent = "  ";
    buf << indent << BasicError::filePosToString(gs, loc) << " " << formattedMessage << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs);
    }
    return buf.str();
}

string ErrorSection::toString(const GlobalState &gs) {
    stringstream buf;
    string indent = "  ";
    if (!this->header.empty()) {
        buf << indent << this->header << endl;
    }
    for (auto &line : this->messages) {
        buf << indent << line.toString(gs) << endl;
    }
    return buf.str();
}

string ComplexError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << BasicError::toString(gs) << endl;

    bool first = true;
    for (auto &section : this->sections) {
        if (!first) {
            buf << endl;
        }
        first = false;
        buf << section.toString(gs);
    }
    return buf.str();
}

ErrorRegion::~ErrorRegion() {
    if (silenceErrors) {
        gs.drainErrors();
    } else {
        gs.flushErrors();
    }
}

} // namespace core
} // namespace ruby_typer
