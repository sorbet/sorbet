# typed: false
class C
  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed

  sig {void}
  def method2
  end
end # error: unexpected token "end of file"
