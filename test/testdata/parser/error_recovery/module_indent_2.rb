# typed: false
class B
  module Inner
# ^^^^^^ error: Hint: this "module" token might not be properly closed
    puts 'hello'
    puts 'hello'

  def method2
  end
end # error: unexpected token "end of file"
