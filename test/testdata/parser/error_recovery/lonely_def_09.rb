# typed: true
class A
  extend T::Sig
  sig {params(x: Integer).void}
  #           ^ error: Unknown argument name `x`
  def self.
# ^^^ parser-error: Hint: this "def" token might not be properly closed
  #       ^ parser-error: Hint: this "." token might not be followed by a method name
end # parser-error: unexpected token "end of file"
