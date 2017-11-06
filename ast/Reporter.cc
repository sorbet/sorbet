#include "Context.h"
#include "spdlog/fmt/ostr.h"

namespace ruby_typer {
namespace ast {

using namespace std;

void Reporter::_error(BasicError error) {
    if (keepErrorsInMemory) {
        errors.emplace_back(make_unique<BasicError>(error));
        return;
    }
    gs_.logger.error("{}", error.toString(gs_));
}

void Reporter::_error(ComplexError error) {
    if (keepErrorsInMemory) {
        errors.emplace_back(make_unique<ComplexError>(error));
        return;
    }
    gs_.logger.error("{}", error.toString(gs_));
}

string Reporter::BasicError::toString(GlobalState &gs) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:";
    } else {
        auto pos = loc.position(gs);
        buf << loc.file.file(gs).path() << ":";
        buf << pos.first.line << ":" << pos.first.column;
        buf << "-";
        buf << pos.second.line << ":" << pos.second.column;
    }
    buf << " " << formatted;
    return buf.str();
}

vector<unique_ptr<ruby_typer::ast::Reporter::BasicError>> Reporter::getAndEmptyErrors() {
    vector<unique_ptr<ruby_typer::ast::Reporter::BasicError>> result;
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
} // namespace ast
} // namespace ruby_typer
