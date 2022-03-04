# typed: false
class F
  sig {void}
  def method1
  end

  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed
    puts 'hello'
end # error: unexpected token "end of file"
