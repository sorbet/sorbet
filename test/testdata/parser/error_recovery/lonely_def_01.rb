# typed: false
class A # parser-error: class definition in method body
  sig {void}
  def
# ^^^ parser-error: Hint: this "def" token might not be followed by a method name
# ^^^ parser-error: Hint: this "def" token might not be properly closed

  sig {void} # parser-error: unexpected token tLCURLY
  def example; end
end
