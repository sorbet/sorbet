# typed: false
class E
  sig {void}
  def method1
  end

  module Inner
# ^^^^^^ parser-error: Hint: this "module" token might not be properly closed
end # parser-error: unexpected token "end of file"
