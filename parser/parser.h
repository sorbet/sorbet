#include <memory>
#include <string>

#include "ruby_parser/diagnostic.hh"

namespace sruby {
namespace parser {

using ast = const void *;

class resultImpl;

/*
 * parser::result contains the result of parsing a piece of Ruby source. It is
 * the owner of all memory allocated during the parse or referenced by the
 * returned objects, and must outlive any references to the AST or diagnostics
 * information.
 */
class result {
public:
    const ruby_parser::diagnostics_t &diagnostics();
    ast ast();

    ~result();

    result(const result &) = delete;
    result(result &&) = default;

private:
    result(std::unique_ptr<resultImpl> &&impl);

    friend result parse_ruby(const std::string &src);

    std::unique_ptr<resultImpl> impl_;
};

result parse_ruby(const std::string &src);
}
};
