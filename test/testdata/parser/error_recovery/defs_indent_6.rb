# typed: false
class F
  sig {void}
  def self.method1
  end

  def self.method2
# ^^^ parser-error: Hint: this "def" token might not be properly closed
    puts 'hello'
end # parser-error: unexpected token "end of file"
