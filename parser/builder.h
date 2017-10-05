#include "ruby_parser/builder.hh"
#include "ruby_parser/driver.hh"

#include <memory>

namespace sruby {
namespace parser {

class builderImpl;
class result;
using ast = const void *;

class builder {
public:
    builder(result &);
    ~builder();

    static ruby_parser::builder interface;

    ast build(ruby_parser::base_driver *driver);

private:
    std::unique_ptr<builderImpl> impl_;
};
};
};
