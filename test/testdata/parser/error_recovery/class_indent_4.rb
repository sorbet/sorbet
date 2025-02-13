# typed: false
class D
  class Inner
# ^^^^^ parser-error: Hint: this "class" token might not be properly closed
    puts 'hello'

  sig {void}
  def method2
  end
end # parser-error: unexpected token "end of file"
