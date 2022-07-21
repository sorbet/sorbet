# typed: false
class A # error: class definition in method body
  sig {void}
  def self.
# ^^^ error: Hint: this "def" token might not be properly closed
  #       ^ error: Hint: this "." token might not be followed by a method name

  sig {void} # error: unexpected token tLCURLY
  def example; end
end
