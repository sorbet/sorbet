# typed: true

class Parent; end

x = T::Class[Parent].new
#                    ^^^ error: mistakes a type for a value
T.reveal_type(x) # error: `T.untyped`
