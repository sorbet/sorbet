# typed: true

# These constant names are never defined anywhere, so the namer has never
# entered them as constant names when it processes `private_constant`.

module Foo
  private_constant :NeverDefinedSymbolArg
  private_constant "NeverDefinedStringArg"
end
