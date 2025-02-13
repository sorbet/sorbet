# typed: false
class D
  module Inner
# ^^^^^^ parser-error: Hint: this "module" token might not be properly closed
    puts 'hello'

  sig {void}
  def method2
  end
end # parser-error: unexpected token "end of file"
