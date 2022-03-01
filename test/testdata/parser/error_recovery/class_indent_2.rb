# typed: false
class B
  class Inner
# ^^^^^ error: Hint: this "class" token might not be properly closed
    puts 'hello'
    puts 'hello'

  def method2
  end
end # error: unexpected token "end of file"
