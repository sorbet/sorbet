#ifndef SORBET_INFERENCE_H
#define SORBET_INFERENCE_H

#include "cfg/CFG.h"
#include <memory>
#include <string>

namespace sorbet::infer {
core::ClassOrModuleRef getCustomerClass(core::Context ctx, const core::TypePtr &type);

class Inference final {
public:
    static bool willRun(core::Context ctx, core::LocOffsets loc, core::MethodRef method);
    static std::unique_ptr<cfg::CFG> run(core::Context ctx, std::unique_ptr<cfg::CFG> cfg);
};
} // namespace sorbet::infer

#endif // SORBET_INFERENCE_H
