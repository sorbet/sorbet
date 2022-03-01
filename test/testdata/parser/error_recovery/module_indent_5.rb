# typed: false
class E
  sig {void}
  def method1
  end

  module Inner
# ^^^^^^ error: Hint: this "module" token might not be properly closed
end # error: unexpected token "end of file"
