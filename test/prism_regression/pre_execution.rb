# typed: false

BEGIN {} # error: Unsupported node type `Preexe`

BEGIN {
  "string1"
  "string2"
} # error: Unsupported node type `Preexe`

BEGIN {
  class ClassInBegin; end
} # error: Unsupported node type `Preexe`

BEGIN {
  def method_in_begin; end
} # error: Unsupported node type `Preexe`

BEGIN {
  CONSTANT_IN_BEGIN = "constant_in_begin"
} # error: Unsupported node type `Preexe`
