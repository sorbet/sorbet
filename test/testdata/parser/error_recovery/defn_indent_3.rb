# typed: false
class C
  def method1
# ^^^ error: Hint: this "def" token might not be properly closed

  sig {void}
  def method2
  end
end # error: unexpected token "end of file"
