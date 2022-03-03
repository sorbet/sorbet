# typed: false
class A
  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed

  def method2
  end
end # error: unexpected token "end of file"
