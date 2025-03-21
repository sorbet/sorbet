# typed: false
class A
  def method1
# ^^^ parser-error: Hint: this "def" token might not be properly closed

  def method2
  end
end # parser-error: unexpected token "end of file"
