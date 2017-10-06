#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include "ast/Context.h"

#include <memory>

namespace sruby {
namespace parser {

class Result;
class Node;

class Builder {
public:
    Builder(sruby::ast::ContextBase &, Result &);
    ~Builder();

    static ruby_parser::builder interface;

    std::unique_ptr<Node> build(ruby_parser::base_driver *driver);

    class Impl;

private:
    std::unique_ptr<Impl> impl_;
};
}; // namespace parser
}; // namespace sruby
