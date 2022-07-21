# typed: true
class A
  extend T::Sig
  sig {params(x: Integer).void}
  #           ^ error: Unknown argument name `x`
  def
# ^^^ error: Hint: this "def" token might not be followed by a method name
# ^^^ error: Hint: this "def" token might not be properly closed
end # error: unexpected token "end of file"
