# typed: true

extend T::Sig

sig { params(x: [Integer, String]).void }
def takes_tuple(x)
end

# too few items
takes_tuple([])
takes_tuple([1])

# mismatch in value types
takes_tuple([1, 1])
takes_tuple(['', 1])

# mismatch in value types and too few items
takes_tuple([''])
