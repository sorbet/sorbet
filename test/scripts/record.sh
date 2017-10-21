DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BIN=$DIR/../../bazel-bin/

$BIN/parser/parse_ast $1 > $1.parser.exp
$BIN/ast/desugar/desugar_ast $1 > $1.desugar.exp
$BIN/namer/namer_print $1 > $1.namer.exp
