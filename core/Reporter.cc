#include "Context.h"
#include "spdlog/fmt/ostr.h"
#include <algorithm>

namespace ruby_typer {
namespace core {

using namespace std;

void Reporter::_error(unique_ptr<BasicError> error) {
    bool isCriticalError = false;
    File::Type source_type = File::Typed;
    if (error->loc.file.exists()) {
        source_type = error->loc.file.file(gs_).source_type;
    }

    switch (error->what) {
        case ErrorClass::Internal:
            isCriticalError = true;
            break;
        case ErrorClass::ParserError:
        case ErrorClass::InvalidMethodSignature:
        case ErrorClass::InvalidTypeDeclaration:
        case ErrorClass::InvalidDeclareVariables:
        case ErrorClass::DuplicateVariableDeclaration:
            // These are always shown, even for untyped source
            break;

        default:
            if (source_type == File::Untyped) {
                return;
            }
    }

    if (isCriticalError) {
        hadCriticalError_ = true;
    }
    auto f = find_if(this->errorHistogram.begin(), this->errorHistogram.end(),
                     [&](auto &el) -> bool { return el.first == (int)error->what; });
    if (f != this->errorHistogram.end()) {
        (*f).second++;
    } else {
        this->errorHistogram.push_back(std::make_pair((int)error->what, 1));
    }
    if (keepErrorsInMemory) {
        errors.emplace_back(move(error));
        return;
    }
    gs_.logger.log(isCriticalError ? spdlog::level::critical : spdlog::level::err, "{}", error->toString(gs_));
}

string Reporter::BasicError::filePosToString(GlobalState &gs, Loc loc) {
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

string Reporter::BasicError::toString(GlobalState &gs) {
    stringstream buf;
    buf << filePosToString(gs, loc) << " " << formatted << " [" << (int)what << "]" << endl;
    if (!loc.is_none()) {
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
    string indent = "  ";
    buf << indent << BasicError::filePosToString(gs, loc) << " " << formattedMessage << endl;
    if (!loc.is_none()) {
        buf << loc.toString(gs);
    }
    return buf.str();
}

string Reporter::ErrorSection::toString(GlobalState &gs) {
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

string Reporter::ComplexError::toString(GlobalState &gs) {
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
} // namespace core
} // namespace ruby_typer
