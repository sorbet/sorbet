#!/bin/bash
# shellcheck disable=SC2086
# ^ Don't complain about word splitting CFLAGS and CXXFLAGS. It's
# traditional semantics for those variables.

set -e

cd "$(dirname "$0")"
here=$(pwd)
objdir="$here/_obj"
mkdir -p "$objdir"

cd .. || exit 1

# Use bazel to fetch our dependencies and pre-build our generated code
bazel build main:sorbet

root="$(bazel info execution_root)"
genfiles="$(bazel info bazel-genfiles)"
absl="$root/external/com_google_absl/"

CFLAGS=""
CFLAGS="$CFLAGS -DNDEBUG"
CFLAGS="$CFLAGS -I$root"
CFLAGS="$CFLAGS -I$absl"
CFLAGS="$CFLAGS -I$root/external/spdlog/include"
CFLAGS="$CFLAGS -I$root/external/rang/include"
CFLAGS="$CFLAGS -I$root/external/lizard"
CFLAGS="$CFLAGS -I$root/external/pdqsort"
CFLAGS="$CFLAGS -I$genfiles"
CFLAGS="$CFLAGS -Oz"

CXXFLAGS="$CFLAGS -std=c++17"

if ! [ "$EMSCRIPTEN" ]; then
    emscripten_root="$(brew --prefix emscripten)"
    export EMSCRIPTEN="$emscripten_root/libexec"
    export LLVM="$emscripten_root/libexec/llvm/bin"
fi

set -x

emcc $CXXFLAGS core/*.cc core/types/*.cc \
     "$genfiles/core/Names_gen.cc" \
     -o $objdir/core.bc

emcc $CXXFLAGS common/*.cc common/os/*.cc \
     -o $objdir/common.bc

emcc $CXXFLAGS "$genfiles/payload/binary/binary.cc" -o $objdir/payload.bc

emcc $CXXFLAGS ast/desugar/*.cc -o $objdir/ast_desugar.bc

emcc $CXXFLAGS dsl/*.cc -o $objdir/dsl.bc

emcc $CXXFLAGS namer/*.cc -o $objdir/namer.bc

emcc $CXXFLAGS resolver/*.cc -o $objdir/resolver.bc

emcc $CXXFLAGS cfg/*.cc cfg/builder/*.cc -o $objdir/cfg.bc

emcc $CXXFLAGS infer/*.cc -o $objdir/infer.bc

emcc $CXXFLAGS parser/*.cc \
     -I "$genfiles/external/parser/include/" \
     -I "$root/external/parser/include/" \
     -I "$genfiles/external/parser/include/ruby_parser" \
     -I "$genfiles/external/parser/cc/grammars/" \
     "$genfiles/parser/diagnostics.cc" \
     "$genfiles/parser/Node_gen.cc" \
     "$genfiles/external/parser/cc/lexer.cc" \
     "$genfiles/external/parser/cc/grammars/typedruby24.cc" \
     "$root/external/parser/cc/"*.cc \
     -o $objdir/parser.bc

emcc $CXXFLAGS \
     "$absl/absl/strings/string_view.cc" \
     "$absl/absl/strings/str_cat.cc" \
     "$absl/absl/strings/numbers.cc" \
     "$absl/absl/strings/internal/memutil.cc" \
     "$absl/absl/base/internal/throw_delegate.cc" \
     -o $objdir/absl.bc

emcc $CXXFLAGS ast/*.cc ast/verifier/*.cc -o $objdir/ast.bc

emcc $CFLAGS "$root/external/lizard/lib"/*.c \
     -DLIZARD_NO_HUFFMAN \
     -o $objdir/lizard.bc

emcc $CXXFLAGS core/serialize/*.cc \
     -o $objdir/serialize.bc

emcc $CXXFLAGS emscripten/main.cc \
     $objdir/common.bc \
     $objdir/core.bc \
     $objdir/payload.bc \
     $objdir/parser.bc \
     $objdir/ast.bc \
     $objdir/ast_desugar.bc \
     $objdir/dsl.bc \
     $objdir/absl.bc \
     $objdir/namer.bc \
     $objdir/resolver.bc \
     $objdir/cfg.bc \
     $objdir/infer.bc \
     $objdir/serialize.bc \
     $objdir/lizard.bc \
     -s EXPORT_NAME="Sorbet" \
     -s MODULARIZE=1 \
     -s 'EXPORTED_FUNCTIONS=["_typecheck"]' \
     -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap"]' \
     -o "$here"/sorbet.html

set +x

echo
echo "Done! Now run these commands to copy the result into sorbet.run:"
echo
echo "    cp emscripten/sorbet.js emscripten/sorbet.wasm ../sorbet.run/docs/"
echo "    git rev-parse HEAD > ../sorbet.run/docs/sha.html"
