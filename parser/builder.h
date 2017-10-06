#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include <memory>

namespace sruby {
namespace parser {

class builderImpl;
class Result;
using ast = const void *;

class Builder {
public:
    Builder(Result &);
    ~Builder();

    static ruby_parser::builder interface;

    ast build(ruby_parser::base_driver *driver);

    class Impl;
private:

    std::unique_ptr<Impl> impl_;
};
};
};
