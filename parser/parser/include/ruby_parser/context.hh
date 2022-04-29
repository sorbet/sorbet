#ifndef RUBY_PARSER_CONTEXT_HH
#define RUBY_PARSER_CONTEXT_HH

#include <optional>
#include <set>
#include <vector>

namespace ruby_parser {

class Context {
public:
    bool inDefined = false;
    bool inKwarg = false;
    bool inArgDef = false;
    bool inDef = false;
    bool inClass = false;
    bool inBlock = false;
    bool inLambda = false;
    bool allowNumparams = false; // Implicitly checks inBlock or inLambda is the last flag to be set

    Context dup() const {
        Context ctx;
        ctx.inDefined = inDefined;
        ctx.inKwarg = inKwarg;
        ctx.inArgDef = inArgDef;
        ctx.inDef = inDef;
        ctx.inClass = inClass;
        ctx.inBlock = inBlock;
        ctx.inLambda = inLambda;
        return ctx;
    }
};

} // namespace ruby_parser

#endif
