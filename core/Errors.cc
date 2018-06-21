#include "core/errors/errors.h"
#include "Context.h"
#include "ErrorQueue.h"
#include "rang.hpp"
#include "spdlog/fmt/ostr.h"
#include <algorithm>

template class std::unique_ptr<sorbet::core::BasicError>;
namespace sorbet {
namespace core {

using namespace std;

constexpr auto ERROR_COLOR = rang::fg::red;
constexpr auto LOW_NOISE_COLOR = rang::fgB::black;
constexpr auto DETAIL_COLOR = rang::fg::yellow;
constexpr auto RESET_COLOR = rang::fg::reset;

constexpr auto INTERPOLATION_COLOR = rang::fg::cyan;

constexpr auto FILE_POS_STYLE = rang::style::underline;
constexpr auto RESET_STYLE = rang::style::reset;

/* used to indicate that previous color should be restored
 * You need to reset color to "previous color that was there", not white.
 * Otherwise you'll cancel color that was wrapping you.
 */
const string REVERT_COLOR_SIGIL = "@@<<``\t\v\b\r\aYOU_FORGOT_TO_CALL restoreColors";

string _replaceAll(const string &inWhat, const string &from, const string &to) {
    size_t cursor = 0;
    string copy = inWhat;
    while ((cursor = copy.find(from, cursor)) != string::npos) {
        copy.replace(cursor, from.length(), to);
        cursor += to.length();
    }
    return copy;
}

string ErrorColors::replaceAll(const string &inWhat, const string &from, const string &to) {
    return _replaceAll(inWhat, from, to);
}

BasicError::BasicError(Loc loc, ErrorClass what, string formatted)
    : loc(loc), what(what), formatted(formatted), isCritical(false) {
    ENFORCE(formatted.back() != '.');
    ENFORCE(formatted.find("\n") == string::npos, formatted, " has a newline in it");
}

string restoreColors(const string &formatted, rang::fg color) {
    stringstream buf;
    buf << color;
    return _replaceAll(formatted, REVERT_COLOR_SIGIL, buf.str());
}

string BasicError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << RESET_STYLE << FILE_POS_STYLE << loc.filePosToString(gs) << RESET_STYLE << ": " << ERROR_COLOR
        << restoreColors(formatted, ERROR_COLOR) << RESET_COLOR << LOW_NOISE_COLOR << " http://go/e/" << what.code
        << RESET_COLOR << '\n';
    if (!loc.is_none()) {
        buf << loc.toString(gs, 2);
    }
    return buf.str();
}

string ErrorLine::toString(const GlobalState &gs, bool color) {
    stringstream buf;
    string indent = "  ";
    buf << indent << FILE_POS_STYLE << loc.filePosToString(gs) << RESET_STYLE << ":";
    if (!formattedMessage.empty()) {
        buf << " ";
        if (color) {
            buf << DETAIL_COLOR << restoreColors(formattedMessage, DETAIL_COLOR) << RESET_COLOR;
        } else {
            buf << restoreColors(formattedMessage, RESET_COLOR);
        }
    }
    buf << '\n';
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
        buf << indent << DETAIL_COLOR << restoreColors(this->header, DETAIL_COLOR) << RESET_COLOR << endl;
    }
    for (auto &line : this->messages) {
        buf << indent << line.toString(gs, coloredLineHeaders);
    }
    return buf.str();
}

string ComplexError::toString(const GlobalState &gs) {
    stringstream buf;
    buf << BasicError::toString(gs);

    for (auto &section : this->sections) {
        buf << section.toString(gs);
    }
    return buf.str();
}

ErrorRegion::~ErrorRegion() {
    gs.errorQueue->flushFile(this->f);
}

ErrorBuilder::ErrorBuilder(const GlobalState &gs, bool willBuild, Loc loc, ErrorClass what)
    : gs(gs), state(willBuild ? State::WillBuild : State::Unreported), loc(loc), what(what) {}

void ErrorBuilder::_setHeader(string &&header) {
    ENFORCE(state == State::WillBuild);
    this->header = move(header);
}

void ErrorBuilder::addErrorSection(ErrorSection &&section) {
    ENFORCE(state == State::WillBuild);
    this->sections.emplace_back(move(section));
}

ErrorBuilder::~ErrorBuilder() {
    if (state != State::WillBuild) {
        return;
    }
    this->gs._error(build());
}

unique_ptr<BasicError> ErrorBuilder::build() {
    ENFORCE(state == State::WillBuild);
    state = State::DidBuild;

    unique_ptr<ComplexError> err =
        make_unique<ComplexError>(this->loc, this->what, move(this->header), move(this->sections));
    if (this->what == errors::Internal::InternalError) {
        err->isCritical = true;
    }
    return err;
}

const string ErrorColors::coloredPatternSigil = "`{}`";
string ErrorColors::coloredPatternReplace = coloredPatternSigil;

void ErrorColors::enableColors() {
    rang::setControlMode(rang::control::Force);
    stringstream buf;
    buf << INTERPOLATION_COLOR << "{}" << REVERT_COLOR_SIGIL;
    coloredPatternReplace = buf.str();
}
void ErrorColors::disableColors() {
    coloredPatternReplace = coloredPatternSigil;
    rang::setControlMode(rang::control::Off);
}
} // namespace core
} // namespace sorbet
