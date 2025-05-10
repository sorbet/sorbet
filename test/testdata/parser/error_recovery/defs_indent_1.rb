# typed: false
class A
  def self.method1
# ^^^ parser-error: Hint: this "def" token might not be properly closed

  def self.method2
  end
end # parser-error: unexpected token "end of file"
