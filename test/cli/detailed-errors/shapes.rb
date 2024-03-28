# typed: true

extend T::Sig

sig { params(x: {a: Integer, b: String}).void }
def takes_shape(x)
end

# too few keys
takes_shape({})
takes_shape({a: 1})

# missing keys
takes_shape({a: 1, c: 1})
takes_shape({c: 1, d: 1})

# mismatch in value types
takes_shape({a: 1, b: 1})
takes_shape({a: '', b: 1})

# mismatch in value types and missing keys
takes_shape({a: '', c: 1})
