# typed: false
class A
  sig {void}
  def self # error: Hint: this "def" token might not be properly closed

  sig {void}
  def example; end
end # error: unexpected token "end of file"
