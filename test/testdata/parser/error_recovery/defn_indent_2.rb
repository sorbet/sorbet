# typed: false
class B
  def method1(abc, xyz)
# ^^^ error: Hint: this "def" token might not be properly closed
    puts 'hello'
    puts 'hello'

  def method2
  end
end # error: unexpected token "end of file"
