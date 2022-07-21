# typed: false
class A # error: class definition in method body
  sig {void}
  def
# ^^^ error: Hint: this "def" token might not be followed by a method name
# ^^^ error: Hint: this "def" token might not be properly closed

  sig {void} # error: unexpected token tLCURLY
  def example; end
end
