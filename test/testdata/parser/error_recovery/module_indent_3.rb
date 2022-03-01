# typed: false
class C
  module Inner
# ^^^^^^ error: Hint: this "module" token might not be properly closed

  sig {void}
  def method2
  end
end # error: unexpected token "end of file"
