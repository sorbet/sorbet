# typed: false

END {} # error: Unsupported node type `Postexe`

END {
  "string1"
  "string2"
} # error: Unsupported node type `Postexe`

END {
  class ClassInEnd; end
} # error: Unsupported node type `Postexe`

END {
  def method_in_end; end
} # error: Unsupported node type `Postexe`

END {
  CONSTANT_IN_END = "constant_in_end"
} # error: Unsupported node type `Postexe`
