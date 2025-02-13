# typed: false
class E
  sig {void}
  def method1
  end

  class Inner
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
end # parser-error: unexpected token "end of file"
