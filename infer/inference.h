#ifndef SRUBY_INFERENCE_H
#define SRUBY_INFERENCE_H

#include "../types/types.h"
#include "cfg/CFG.h"
#include <memory>
#include <string>

namespace ruby_typer {
namespace infer {
class Inference {
    static void run(ast::Context ctx, cfg::CFG &cfg);
};
} // namespace infer
} // namespace ruby_typer

#endif // SRUBY_INFERENCE_H
