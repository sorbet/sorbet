#include <iostream>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>

using llvm::BitcodeWriter;
using llvm::LLVMContext;
using llvm::Module;
using llvm::SMDiagnostic;
using llvm::StringRef;

namespace {

// Mark all `sorbet_` functions defined in the payload with `internal` linkage, so that they are available for inlining,
// but are GC'd if they aren't used.
//
// NOTE: marking these functions as `internal` is the same as marking them `static` in codegen-payload.c. However, if we
// were to mark them all static, the compiler would prune them all out and we'd be left with an empty payload. We want
// these functions to be private to code-generation, and there's just not a great way to say that in the c source.
void setSorbetFunctionLinkage(Module &module) {
    for (auto &fun : module.functions()) {
        auto name = fun.getName();

        if (fun.isDeclaration()) {
            // When given ruby code like the following:
            //   if cond
            //     ABC.new(1)
            //   else
            //     DEF.new(1)
            //   end
            // We emit something like the following:
            //   %1 = <cond>
            //   br i1 %1, label %BB1, label %BB2
            //   BB1:
            //   class = sorbet_i_getRubyClass("ABC")
            //   sorbet_i_send(class, "new")
            //   BB2:
            //   class = sorbet_i_getRubyClass("DEF")
            //   sorbet_i_send(class, "new")
            // LLVM notices that ABC and DEF are string constants of the same length, and replaces this with:
            //   class = sorbet_i_getRubyClass(phi "ABC", "DEF")
            //   sorbet_i_send(class, "new")
            // This causes an issue, because sorbet_i_getRubyClass can only handle constants, and returns undef when
            // it encounters the phi node.
            // By marking sorbet_i_getRubyClass and sorbet_i_getRubyConstant as nomerge, we are telling LLVM not to
            // attempt to dedup calls to them, and emit the original LLVM code instead.
            if (name.equals("sorbet_i_getRubyClass") || name.equals("sorbet_i_getRubyConstant")) {
                fun.addFnAttr(llvm::Attribute::NoMerge);
            }
            continue;
        }

        // Keep the special keep-alive functions as external, as they will be explicitly removed by a pass late in
        // the compiler's pipeline.
        if (name.startswith("sorbet_exists_to_keep_alive_") && fun.hasExternalLinkage()) {
            continue;
        }

        // Set all of the `external` (default linkage) functions defined in the payload to `internal` linkage, as any
        // other functions have more specific linkage set explicitly in the source.
        if (name.startswith("sorbet_") && fun.hasExternalLinkage()) {
            fun.setLinkage(llvm::GlobalValue::InternalLinkage);
        }
    }
}

// Mark ruby constant globals explicitly as constants -- we know that values like `rb_cClass` etc are never going to
// change once the vm has initialized.
//
// quoting spec:
// > LLVM explicitly allows declarations of global variables to be marked constant, even if the final definition of the
// > global is not.  This capability can be used to enable slightly better optimization of the program, but requires the
// > language definition to guarantee that optimizations based on the ‘constantness’ are valid for the translation units
// > that do not include the definition.
void markRubyConstants(Module &module) {
    for (auto &cnst : module.globals()) {
        auto name = cnst.getName();

        // TODO: this should be expanded to `rb_e` and `rb_m` as well
        if (name.startswith("rb_c") && cnst.getUnnamedAddr() == llvm::GlobalValue::UnnamedAddr::Local) {
            cnst.setConstant(true);
        }

        // See the definition of `SORBET_CONSTANT` in the codegen payload.
        if (name.startswith("sorbet_") &&
            cnst.getVisibility() == llvm::GlobalValue::VisibilityTypes::HiddenVisibility) {
            cnst.setVisibility(llvm::GlobalValue::VisibilityTypes::DefaultVisibility);
            cnst.setLinkage(llvm::GlobalVariable::LinkageTypes::InternalLinkage);
        }
    }
}

void clearModuleFlags(Module &module) {
    // Remove the llvm.module.flags debug metadata, as it will raise verification errors when inlining functions defined
    // in the payload if it's present.
    if (auto *flags = module.getModuleFlagsMetadata()) {
        flags->eraseFromParent();
    }

    // Remove the llvm.ident metadata as we don't really need to include information about the version of clang we used
    // to generate the payload.
    if (auto *ident = module.getNamedMetadata("llvm.ident")) {
        ident->eraseFromParent();
    }
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: postprocess_payload <payload.bc> <output.bc>" << std::endl;
        return 1;
    }

    LLVMContext ctx;
    SMDiagnostic errors;
    auto module = llvm::parseIRFile(StringRef(argv[1]), errors, ctx);

    setSorbetFunctionLinkage(*module);
    markRubyConstants(*module);
    clearModuleFlags(*module);

    // We strip out information that makes debug info work in clearModuleFlags, so strip everything out as that will
    // happen when the payload is loaded by sorbet.
    llvm::StripDebugInfo(*module);

    // Sanity check the changes we make
    if (llvm::verifyModule(*module, &llvm::errs())) {
        return 1;
    }

    std::error_code ec;
    llvm::raw_fd_ostream out(argv[2], ec, llvm::sys::fs::OF_None);
    llvm::WriteBitcodeToFile(*module, out);
    out.flush();

    return 0;
}
