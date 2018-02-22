#include "core/errors/errors.h"
#include "Context.h"
#include "ErrorQueue.h"
#include "rang.hpp"
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
        buf << loc.file.data(gs).path() << ":";
        buf << pos.first.line;
        if (pos.second.line != pos.first.line) {
            buf << "-";
            buf << pos.second.line;
        }
        buf << ":";
    }
    return buf.str();
}

string BasicError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << rang::style::reset << rang::style::bold << rang::fg::red << filePosToString(gs, loc) << rang::style::reset
        << rang::fg::red << " " << formatted << " [" << what.code << "]" << rang::fg::reset << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs, 2);
    }
    return buf.str();
}

string ErrorLine::toString(const GlobalState &gs) {
    stringstream buf;
    string indent = "  ";
    buf << indent << rang::style::bold << BasicError::filePosToString(gs, loc) << rang::style::reset << " "
        << formattedMessage << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs, 2);
    }
    return buf.str();
}

string ErrorSection::toString(const GlobalState &gs) {
    stringstream buf;
    string indent = "  ";
    if (!this->header.empty()) {
        buf << rang::fg::yellow << indent << this->header << rang::fg::reset;
    }
    bool first = true;
    for (auto &line : this->messages) {
        if (!first) {
            buf << endl;
        }
        first = false;
        buf << indent << line.toString(gs);
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
    ErrorQueueMessage msg;
    msg.kind = silenceErrors ? ErrorQueueMessage::Kind::Drop : ErrorQueueMessage::Kind::Flush;
    msg.whatFile = this->f;
    gs.errorQueue->queue.push(move(msg), 1);
}

} // namespace core
} // namespace ruby_typer
