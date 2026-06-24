# typed: true

def unknown(sym); end

class Demo
  # Unrecognized method modifiers should be inert, and not disrupt the behaviour of other modifiers.

  private unknown def private_unknown; end

  unknown private def unknown_private; end
end

x = Demo.new

x.private_unknown
# ^^^^^^^^^^^^^^^ error: Non-private call to private method `private_unknown` on `Demo`
x.unknown_private
# ^^^^^^^^^^^^^^^ error: Non-private call to private method `unknown_private` on `Demo`
