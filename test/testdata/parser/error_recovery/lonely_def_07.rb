# typed: false
class A # parser-error: class definition in method body
  sig {void}
  def self.
# ^^^ parser-error: Hint: this "def" token might not be properly closed
  #       ^ parser-error: Hint: this "." token might not be followed by a method name

  sig {void} # parser-error: unexpected token tLCURLY
  def example; end
end
