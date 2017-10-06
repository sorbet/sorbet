#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include "ast/Context.h"

#include <memory>

namespace sruby {
namespace parser {

class builderImpl;
class Result;
using ast = const void *;

class Builder {
public:
    Builder(sruby::ast::ContextBase &, Result &);
    ~Builder();

    static ruby_parser::builder interface;

    ast build(ruby_parser::base_driver *driver);

    class Impl;

private:
    std::unique_ptr<Impl> impl_;
};
}; // namespace parser
}; // namespace sruby
