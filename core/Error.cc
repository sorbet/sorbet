// has to go first as it violates our poisons
#include "rang.hpp"

#include "absl/strings/str_replace.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/GlobalState.h"
#include "core/errors/errors.h"
#include "core/lsp/QueryResponse.h"
#include <algorithm>
#include <sstream>

template class std::unique_ptr<sorbet::core::Error>;
namespace sorbet::core {

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
constexpr string_view REVERT_COLOR_SIGIL = "@@<<``\t\v\b\r\aYOU_FORGOT_TO_CALL restoreColors"sv;

string _replaceAll(string_view inWhat, string_view from, string_view to) {
    return absl::StrReplaceAll(inWhat, {{from, to}});
}

string ErrorColors::replaceAll(string_view inWhat, string_view from, string_view to) {
    return _replaceAll(inWhat, from, to);
}

bool Error::isCritical() const {
    return this->what.minLevel == StrictLevel::Internal;
}

string restoreColors(string_view formatted, rang::fg color) {
    stringstream buf;
    buf << color;
    return _replaceAll(formatted, REVERT_COLOR_SIGIL, buf.str());
}

string ErrorLine::toString(const GlobalState &gs, bool color) const {
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

    if (loc.exists()) {
        buf << '\n' << loc.toStringWithTabs(gs, 2);
    }
    return buf.str();
}

string ErrorSection::toString(const GlobalState &gs) const {
    stringstream buf;
    string indent = "  ";
    bool coloredLineHeaders = true;
    bool skipEOL = false;
    if (!this->header.empty()) {
        coloredLineHeaders = false;
        buf << indent << DETAIL_COLOR << restoreColors(this->header, DETAIL_COLOR) << RESET_COLOR;
    } else {
        skipEOL = true;
    }
    for (auto &line : this->messages) {
        if (!skipEOL) {
            buf << '\n';
        }
        skipEOL = false;
        buf << indent << line.toString(gs, coloredLineHeaders);
    }
    return buf.str();
}

string Error::toString(const GlobalState &gs) const {
    stringstream buf;
    buf << RESET_STYLE << FILE_POS_STYLE << loc.filePosToString(gs) << RESET_STYLE << ": " << ERROR_COLOR
        << restoreColors(header, ERROR_COLOR) << RESET_COLOR << LOW_NOISE_COLOR << " " << gs.errorUrlBase << what.code
        << RESET_COLOR;
    if (loc.exists()) {
        buf << '\n' << loc.toStringWithTabs(gs, 2);
    }

    for (auto &section : this->sections) {
        buf << '\n' << section.toString(gs);
    }
    return buf.str();
}

ErrorRegion::~ErrorRegion() {
    gs.errorQueue->markFileForFlushing(this->f);
}

ErrorBuilder::ErrorBuilder(const GlobalState &gs, bool willBuild, Loc loc, ErrorClass what)
    : gs(gs), state(willBuild ? State::WillBuild : State::Unreported), loc(loc), what(what) {
    ENFORCE(willBuild || what.minLevel != StrictLevel::Internal);
}

void ErrorBuilder::_setHeader(string &&header) {
    ENFORCE(state == State::WillBuild);
    this->header = move(header);
}

void ErrorBuilder::addErrorSection(ErrorSection &&section) {
    ENFORCE(state == State::WillBuild);
    this->sections.emplace_back(move(section));
}

void ErrorBuilder::addAutocorrect(AutocorrectSuggestion &&autocorrect) {
    ENFORCE(state == State::WillBuild);
    u4 n = autocorrect.loc.endPos() - autocorrect.loc.beginPos();
    if (gs.autocorrect) {
        auto verb = n == 0 ? "Inserted" : "Replaced with";
        addErrorSection(ErrorSection("Autocorrect: Done",
                                     {ErrorLine::from(autocorrect.loc, "{} `{}`", verb, autocorrect.replacement)}));
    } else {
        auto verb = n == 0 ? "Insert" : "Replace with";
        addErrorSection(ErrorSection("Autocorrect: Use `-a` to autocorrect",
                                     {ErrorLine::from(autocorrect.loc, "{} `{}`", verb, autocorrect.replacement)}));
    }
    this->autocorrects.emplace_back(move(autocorrect));
}

// This will sometimes be bypassed in lieue of just calling build() so put your
// logic in build() instead.
ErrorBuilder::~ErrorBuilder() {
    if (state == State::DidBuild) {
        return;
    }
    this->gs._error(build());
}

unique_ptr<Error> ErrorBuilder::build() {
    ENFORCE(state != State::DidBuild);
    bool isSilenced = state == State::Unreported;
    state = State::DidBuild;

    unique_ptr<Error> err = make_unique<Error>(this->loc, this->what, move(this->header), move(this->sections),
                                               move(this->autocorrects), isSilenced);
    return err;
}

string ErrorColors::coloredPatternReplace = (string)coloredPatternSigil;

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

} // namespace sorbet::core
