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
CFLAGS="$CFLAGS -I$root"
CFLAGS="$CFLAGS -I$absl"
CFLAGS="$CFLAGS -I$root/external/spdlog/include"
CFLAGS="$CFLAGS -I$root/external/rang/include"
CFLAGS="$CFLAGS -I$root/external/lizard"
CFLAGS="$CFLAGS -I$genfiles"
CFLAGS="$CFLAGS -Oz"

CXXFLAGS="$CFLAGS -std=c++14"

set -x

emcc $CXXFLAGS core/*.cc core/types/*.cc \
     "$genfiles/core/Names_gen.cc" \
     -o _obj/core.bc

emcc $CXXFLAGS common/*.cc common/os/*.cc \
     -o _obj/common.bc

emcc $CXXFLAGS "$genfiles/payload/binary/binary.cc" -o _obj/payload.bc

emcc $CXXFLAGS ast/desugar/*.cc -o _obj/ast_desugar.bc

emcc $CXXFLAGS dsl/*.cc -o _obj/dsl.bc

emcc $CXXFLAGS namer/*.cc -o _obj/namer.bc

emcc $CXXFLAGS resolver/*.cc -o _obj/resolver.bc

emcc $CXXFLAGS cfg/*.cc cfg/builder/*.cc -o _obj/cfg.bc

emcc $CXXFLAGS infer/*.cc -o _obj/infer.bc

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
     -o _obj/parser.bc

emcc $CXXFLAGS \
     "$absl/absl/strings/string_view.cc" \
     "$absl/absl/strings/str_cat.cc" \
     "$absl/absl/strings/numbers.cc" \
     "$absl/absl/strings/internal/memutil.cc" \
     "$absl/absl/base/internal/throw_delegate.cc" \
     -o _obj/absl.bc

emcc $CXXFLAGS ast/*.cc ast/verifier/*.cc -o _obj/ast.bc

emcc $CXXFLAGS "$root/external/lizard/lib"/*.c \
     -DLIZARD_NO_HUFFMAN \
     -o _obj/lizard.bc

emcc $CXXFLAGS core/serialize/*.cc \
     -o _obj/serialize.bc

emcc $CXXFLAGS emscripten/main.cc \
     _obj/common.bc \
     _obj/core.bc \
     _obj/payload.bc \
     _obj/parser.bc \
     _obj/ast.bc \
     _obj/ast_desugar.bc \
     _obj/dsl.bc \
     _obj/absl.bc \
     _obj/namer.bc \
     _obj/resolver.bc \
     _obj/cfg.bc \
     _obj/infer.bc \
     _obj/serialize.bc \
     _obj/lizard.bc \
     -s EXPORT_NAME="Sorbet" \
     -s MODULARIZE=1 \
     -s 'EXPORTED_FUNCTIONS=["_typecheck"]' \
     -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap"]' \
     -o "$here"/sorbet.html
