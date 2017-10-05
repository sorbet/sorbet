#include "parser/parser.h"
#include "parser/builder.h"

#include "ruby_parser/driver.hh"

namespace sruby {
namespace parser {


using std::string;
using std::unique_ptr;

class resultImpl {
public:
    friend class result;

    resultImpl(const string &src) : driver(src, builder) {}

    ruby_parser::typedruby24 driver;
};

const ruby_parser::diagnostics_t &result::diagnostics() {
    return impl_->driver.diagnostics;
}

ast result::ast() {
    return nullptr;
}

result::result(std::unique_ptr<resultImpl> &&impl) : impl_(std::move(impl)) {}
result::~result() {}

result parse_ruby(const string &src) {
    unique_ptr<resultImpl> impl(new resultImpl(src));
    impl->driver.parse(nullptr);
    return result(std::move(impl));
}
};
};
