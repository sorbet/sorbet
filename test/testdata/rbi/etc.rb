# typed: true

T.reveal_type(Etc.getpwuid) # error: Revealed type: `T.nilable(Etc::Passwd)`
T.reveal_type(Etc.getpwuid(10)) # error: Revealed type: `T.nilable(Etc::Passwd)`

Etc.getpwuid("foo") # error: Expected `Integer` but found `String("foo")`

T.reveal_type(Etc.uname) # error: Revealed type: `T::Hash[Symbol, String]`
