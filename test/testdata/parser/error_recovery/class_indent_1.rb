# typed: false
class A
  class Inner
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed

  def method2
  end
end # parser-error: unexpected token "end of file"
