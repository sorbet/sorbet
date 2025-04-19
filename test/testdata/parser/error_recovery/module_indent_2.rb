# typed: false
class B
  module Inner
# ^^^^^^ parser-error: Hint: this "module" token might not be properly closed
    puts 'hello'
    puts 'hello'

  def method2
  end
end # parser-error: unexpected token "end of file"
