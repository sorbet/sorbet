#ifndef SORBET_INFERENCE_H
#define SORBET_INFERENCE_H

#include "cfg/CFG.h"
#include <memory>
#include <string>

namespace ruby_typer {
namespace infer {
class Inference final {
public:
    static std::unique_ptr<cfg::CFG> run(core::Context ctx, std::unique_ptr<cfg::CFG> cfg);
};
} // namespace infer
} // namespace ruby_typer

#endif // SORBET_INFERENCE_H
