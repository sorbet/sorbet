# typed: strict
# enable-experimental-rbs-comments: true

extend T::Sig

class P1; end
class P2; end
class P3; end
class P4; end
class P5; end
class P6; end

class AttrsMultiline
  #: Hash[
  #| -> void
  #  ^^ error: Failed to parse RBS type (expected a token `pARROW`)
  attr_reader :parse_error1 # error: The method `parse_error1` does not have a `sig`

  #: Hash[
  #| P2
  #|   -> void
  #    ^^ error: Failed to parse RBS type (expected a token `pARROW`)
  attr_reader :parse_error2; T.unsafe(nil) # error: The method `parse_error2` does not have a `sig`

  #: [
  #| P1,
  #|  P2
  #| -> void
  #  ^^ error: Failed to parse RBS type (expected ',' or ']' after type parameter, got pARROW)
  attr_reader :parse_error3 # error: The method `parse_error3` does not have a `sig`

  #:
  #|   -> String
  #    ^^ error: Using a method signature on an accessor is not allowed, use a bare type instead
  attr_reader :parse_error4 # error: The method `parse_error4` does not have a `sig`

  # some comment
  #: P1
  #| | P2
  attr_reader :attr1

  #: String
# ^^^^^^^^^ error: Unused type annotation. No method def before next annotation
  #: P1 |
  #| P2
  attr_reader :attr2

  #: [P1,
  #|
  #| P10
  #  ^^^ error: Unable to resolve constant `P10`
  #| ]
  attr_reader :attr3

  #: Hash[
  #| P1,
  #| {
  #|   p2s:
  #|   [
  #|     P2,
  #|     P2
  #|   ]
  #| }
  #|]
  attr_reader :attr4

  #: -> void
  def initialize
    @parse_error1 = T.let(T.unsafe(nil), Integer)
    @parse_error2 = T.let(T.unsafe(nil), Integer)
    @parse_error3 = T.let(T.unsafe(nil), Integer)
    @parse_error4 = T.let(T.unsafe(nil), Integer)
    @attr1 = T.let(T.unsafe(nil), T.any(P1, P2))
    @attr2 = T.let(T.unsafe(nil), T.any(P1, P2))
    @attr3 = T.let(T.unsafe(nil), [P1, P2])
    @attr4 = T.let(T.unsafe(nil), T::Hash[P1, {p2s: [P2, P2]}])
  end
end
