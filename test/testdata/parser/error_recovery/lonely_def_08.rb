# typed: true
class A
  extend T::Sig
  sig {params(x: Integer).void}
  #           ^ error: Unknown argument name `x`
  def self # parser-error: Hint: this "def" token might not be properly closed
end # parser-error: unexpected token "end of file"
