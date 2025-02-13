# typed: false
class F
  sig {void}
  def method1
  end

  module Inner
# ^^^^^^ parser-error: Hint: this "module" token might not be properly closed
    puts 'hello'
end # parser-error: unexpected token "end of file"
