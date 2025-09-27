# typed: true
class A
  extend T::Sig
  sig {params(x: Integer).void}
  #           ^ error: Unknown parameter name `x`
  def self.
# ^^^ error: Hint: this "def" token might not be properly closed
  #       ^ error: Hint: this "." token might not be followed by a method name
end # error: unexpected token "end of file"
