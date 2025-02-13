# typed: false
class A
  sig {void}
  def self # parser-error: Hint: this "def" token might not be properly closed

  sig {void}
  def example; end
end # parser-error: unexpected token "end of file"
