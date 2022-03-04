# typed: false
class E
  sig {void}
  def self.method1
  end

  def self.method2
# ^^^ error: Hint: this "def" token might not be properly closed
end # error: unexpected token "end of file"
