# typed: false
class D
  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed
    puts 'hello'

  sig {void}
  def method2
  end
end # error: unexpected token "end of file"
