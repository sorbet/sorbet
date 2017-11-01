#ifndef SRUBY_INFERENCE_H
#define SRUBY_INFERENCE_H

#include "../ast/ast.h"
#include "../cfg/CFG.h"
#include <memory>
#include <string>

namespace ruby_typer {
namespace infer {
class Inference {
public:
    static void run(ast::Context ctx, std::unique_ptr<cfg::CFG> &cfg);
};
} // namespace infer
} // namespace ruby_typer

#endif // SRUBY_INFERENCE_H
