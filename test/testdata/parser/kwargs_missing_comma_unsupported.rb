# typed: true

# In some cases, but not all, we are able to recover from parse errors if the comma between keyword args is missing.
# This test contains some unsupported cases, along with their current behavior.
class A # error: class definition in method body
  # Optional kwargs with missing commas don't work. (This may be hard to implement well, though perhaps we could
  # support a few ad hoc cases?)
  def example1(slug:, token:, merchant: 3 request:)
                                        # ^^^^^^^ error: unexpected token tIDENTIFIER
  end

  # For some reason, things don't work correctly when the comma is missing and the next kwarg is on the next line.
  def example2(slug, token:, merchant:
                             request:)
                                  # ^ error: unexpected token ":"
  end
end
