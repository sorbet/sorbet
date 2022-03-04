# typed: false
class A
  module Inner
# ^^^^^^ error: Hint: this "module" token might not be properly closed

  def method2
  end
end # error: unexpected token "end of file"
