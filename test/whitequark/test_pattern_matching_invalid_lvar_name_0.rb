# typed: true

case a; in a?:; end # parser-error: a? is not allowed as a local variable name
