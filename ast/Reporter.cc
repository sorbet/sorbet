#include "Context.h"
#include "spdlog/fmt/ostr.h"
#include <cstring> // memchr

namespace ruby_typer {
namespace ast {

using namespace std;

void Reporter::_error(Error error) {
    if (keepErrorsInMemory) {
        errors.push_back(error);
        return;
    }
    ctx_.logger.error("{}", error.toString(ctx_));
}

void Reporter::_error(ComplexError error) {
    ctx_.logger.error("{}", error.toString(ctx_));
}

std::string Reporter::Error::toString(GlobalState &ctx) {
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

std::vector<Reporter::Error> Reporter::getAndEmptyErrors() {
    return errors;
}

std::string Reporter::ErrorLine::toString(GlobalState &ctx) {
    stringstream buf;
    if (loc.is_none()) {
        buf << "???:"
            << " " << formattedMessage;
    } else {
        auto pos = loc.position(ctx);
        buf << loc.file.file(ctx).path() << ":";
        buf << pos.first.line << ":" << pos.first.column;
        buf << "-";
        buf << pos.second.line << ":" << pos.second.column;
        buf << " " << formattedMessage << endl;
        UTF8Desc source = loc.file.file(ctx).source();
        const char *i = source.from;
        int lineCount = 0;
        while (lineCount < pos.first.line - 1) {
            i = (const char *)memchr(i + 1, '\n', source.to - (i - source.from));
            lineCount++;
        }
        const char *j = i;
        while (lineCount < pos.second.line) {
            const char *nj = (const char *)memchr(j + 1, '\n', source.to - (j - source.from));
            if (nj != nullptr) {
                j = nj;
            } else {
                break;
            }
            lineCount++;
        }
        string outline(i + 1, j - i - 1);
        buf << outline;
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
    buf << this->header << endl;
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
