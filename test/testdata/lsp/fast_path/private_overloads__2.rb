# typed: true

def example
end

po1 = PrivateOverloads.new.foo # error: Non-private call to private method
T.reveal_type(po1) # error: Revealed type: `NilClass`

po2 = PrivateOverloads.new.foo(0) # error: Non-private call to private method
T.reveal_type(po2) # error: Revealed type: `Integer`
