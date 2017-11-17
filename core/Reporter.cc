#include "Context.h"
#include "spdlog/fmt/ostr.h"

namespace ruby_typer {
namespace core {

using namespace std;

void Reporter::_error(unique_ptr<BasicError> error) {
    bool isCriticalError = (error->what == ErrorClass::Internal);
    if (isCriticalError) {
        hadCriticalError_ = true;
    }
    if (keepErrorsInMemory) {
        errors.emplace_back(move(error));
        return;
    }
    gs_.logger.log(isCriticalError ? spdlog::level::critical : spdlog::level::err, "{}", error->toString(gs_));
}

string Reporter::BasicError::toString(GlobalState &gs) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:"
            << " " << formatted;
    } else {
        auto pos = loc.position(gs);
        buf << loc.file.file(gs).path() << ":";
        buf << pos.first.line;
        if (pos.second.line != pos.first.line) {
            buf << "-";
            buf << pos.second.line;
        }
        buf << " " << formatted << endl;
        buf << loc.toString(gs);
    }
    return buf.str();
}

vector<unique_ptr<ruby_typer::core::Reporter::BasicError>> Reporter::getAndEmptyErrors() {
    vector<unique_ptr<ruby_typer::core::Reporter::BasicError>> result;
    result.swap(errors);
    return result;
}

string Reporter::ErrorLine::toString(GlobalState &gs) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:"
            << " " << formattedMessage;
    } else {
        auto pos = loc.position(gs);
        buf << loc.file.file(gs).path() << ":";
        buf << pos.first.line;
        if (pos.second.line != pos.first.line) {
            buf << "-";
            buf << pos.second.line;
        }
        buf << " " << formattedMessage << endl;
        buf << loc.toString(gs);
    }

    return buf.str();
}

string Reporter::ErrorSection::toString(GlobalState &gs) {
    stringstream buf;
    buf << this->header << endl;
    for (auto &line : this->messages) {
        buf << line.toString(gs) << endl;
    }
    return buf.str();
}

string Reporter::ComplexError::toString(GlobalState &gs) {
    stringstream buf;
    if (!this->loc.is_none()) {
        buf << this->loc.toString(gs) << endl;
    }
    buf << '[' << (int)this->what << "] " << this->formatted << endl;
    bool first = true;
    for (auto &line : this->sections) {
        if (!first)
            buf << endl;
        first = false;
        buf << line.toString(gs);
    }
    return buf.str();
}
} // namespace core
} // namespace ruby_typer
