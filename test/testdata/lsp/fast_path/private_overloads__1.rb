# typed: __STDLIB_INTERNAL
# spacer for exclude-from-file-update

class PrivateOverloads
  extend T::Sig

  sig {returns(NilClass)}
  sig {params(x: Integer).returns(Integer)}
  private def foo(x=nil); end
end

po1 = PrivateOverloads.new.foo # error: Non-private call to private method
T.reveal_type(po1) # error: Revealed type: `NilClass`
#             ^ hover: NilClass

# Must be at least one `hover` in this file to trigger the
# LSPTypechecker::retypecheck codepath (fast path with no edit)

po2 = PrivateOverloads.new.foo(0) # error: Non-private call to private method
T.reveal_type(po2) # error: Revealed type: `Integer`
