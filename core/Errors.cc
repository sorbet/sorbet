#include "core/errors/errors.h"
#include "Context.h"
#include "ErrorQueue.h"
#include "rang.hpp"
#include "spdlog/fmt/ostr.h"
#include <algorithm>

namespace ruby_typer {
namespace core {

using namespace std;

constexpr auto ERROR_COLOR = rang::fg::red;
constexpr auto LOW_NOISE_COLOR = rang::fgB::black;
constexpr auto DETAIL_COLOR = rang::fg::yellow;
constexpr auto RESET_COLOR = rang::fg::reset;

constexpr auto FILE_POS_STYLE = rang::style::underline;
constexpr auto RESET_STYLE = rang::style::reset;

BasicError::BasicError(Loc loc, ErrorClass what, std::string formatted)
    : loc(loc), what(what), formatted(formatted), isCritical(false) {
    //    ENFORCE(formatted.find("\n") == std::string::npos, formatted, " has a newline in it");
    // TODO: @dmitry: temporarily disable.
}

string BasicError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << RESET_STYLE << FILE_POS_STYLE << loc.filePosToString(gs) << RESET_STYLE << ": " << ERROR_COLOR << formatted
        << RESET_COLOR << LOW_NOISE_COLOR << " http://go/e/" << what.code << RESET_COLOR << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs, 2);
    }
    return buf.str();
}

string ErrorLine::toString(const GlobalState &gs, bool color) {
    stringstream buf;
    string indent = "  ";
    buf << indent << FILE_POS_STYLE << loc.filePosToString(gs) << RESET_STYLE << ": ";
    if (color) {
        buf << DETAIL_COLOR;
    }
    buf << formattedMessage << RESET_COLOR << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs, 2);
    }
    return buf.str();
}

string ErrorSection::toString(const GlobalState &gs) {
    stringstream buf;
    string indent = "  ";
    bool coloredLineHeaders = true;
    if (!this->header.empty()) {
        coloredLineHeaders = false;
        buf << indent << DETAIL_COLOR << this->header << RESET_COLOR;
    }
    bool first = true;
    for (auto &line : this->messages) {
        if (!first) {
            buf << endl;
        }
        first = false;
        buf << indent << line.toString(gs, coloredLineHeaders);
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
