# typed: false
class D
  def self.method1
# ^^^ error: Hint: this "def" token might not be properly closed
    puts 'hello'

  sig {void}
  def self.method2
  end
end # error: unexpected token "end of file"
