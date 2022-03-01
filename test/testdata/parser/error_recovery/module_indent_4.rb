# typed: false
class D
  module Inner
# ^^^^^^ error: Hint: this "module" token might not be properly closed
    puts 'hello'

  sig {void}
  def method2
  end
end # error: unexpected token "end of file"
