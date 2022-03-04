# typed: false
class E
  sig {void}
  def method1
  end

  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed
end # error: unexpected token "end of file"
