# typed: true

# This being typed: true means that this is not the canonical loc of `Child`

class Child < Parent # error: Missing definition for abstract method
end
