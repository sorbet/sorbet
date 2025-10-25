# typed: true

class Parent; end
module IFoo; end

x = T::Class[Parent].new
#                    ^^^ error: mistakes a type for a value
T.reveal_type(x) # error: `T.untyped`

x = T::Class[IFoo].new
#                  ^^^ error: mistakes a type for a value
T.reveal_type(x) # error: `T.untyped`

x = T::Module[Parent].new
#                     ^^^ error: mistakes a type for a value
T.reveal_type(x) # error: `T.untyped`

x = T::Module[IFoo].new
#                   ^^^ error: mistakes a type for a value
T.reveal_type(x) # error: `T.untyped`
