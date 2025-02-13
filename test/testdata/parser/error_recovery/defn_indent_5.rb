# typed: false
class E
  sig {void}
  def method1
  end

  def method2
# ^^^ parser-error: Hint: this "def" token might not be properly closed
end # parser-error: unexpected token "end of file"
