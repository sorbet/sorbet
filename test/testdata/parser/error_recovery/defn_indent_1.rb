# typed: false
class A
  def method1
# ^^^ error: Hint: this "def" token might not be properly closed

  def method2
  end
end # error: unexpected token "end of file"
