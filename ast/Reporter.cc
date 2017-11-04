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
    ctx_.logger.error("{}", error.toString(ctx_));
}

void Reporter::_error(ComplexError error) {
    if (keepErrorsInMemory) {
        errors.emplace_back(make_unique<ComplexError>(error));
        return;
    }
    ctx_.logger.error("{}", error.toString(ctx_));
}

std::string Reporter::BasicError::toString(GlobalState &ctx) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:";
    } else {
        auto pos = loc.position(ctx);
        buf << loc.file.file(ctx).path() << ":";
        buf << pos.first.line << ":" << pos.first.column;
        buf << "-";
        buf << pos.second.line << ":" << pos.second.column;
    }
    buf << " " << formatted;
    return buf.str();
}

std::vector<unique_ptr<ruby_typer::ast::Reporter::BasicError>> Reporter::getAndEmptyErrors() {
    std::vector<unique_ptr<ruby_typer::ast::Reporter::BasicError>> result;
    result.swap(errors);
    return result;
}

std::string Reporter::ErrorLine::toString(GlobalState &ctx) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:"
            << " " << formattedMessage;
    } else {
        auto pos = loc.position(ctx);
        buf << loc.file.file(ctx).path() << ":";
        buf << pos.first.line;
        if (pos.second.line != pos.first.line) {
            buf << "-";
            buf << pos.second.line;
        }
        buf << " " << formattedMessage << endl;
        buf << loc.toString(ctx);
    }

    return buf.str();
}

std::string Reporter::ErrorSection::toString(GlobalState &ctx) {
    stringstream buf;
    buf << this->header << endl;
    for (auto &line : this->messages) {
        buf << line.toString(ctx) << endl;
    }
    return buf.str();
}

std::string Reporter::ComplexError::toString(GlobalState &ctx) {
    stringstream buf;
    if (!this->loc.is_none()) {
        buf << this->loc.toString(ctx) << endl;
    }
    buf << '[' << (int)this->what << "] " << this->formatted << endl;
    bool first = true;
    for (auto &line : this->sections) {
        if (!first)
            buf << endl;
        first = false;
        buf << line.toString(ctx);
    }
    return buf.str();
}
} // namespace ast
} // namespace ruby_typer
