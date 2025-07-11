# typed: strict
# enable-experimental-rbs-comments: true

# let

let1, *let2 = ARGV #: Array[String]
T.reveal_type(let1) # error: Revealed type: `T.nilable(String)`
T.reveal_type(let2) # error: Revealed type: `T::Array[String]`

(let3, *let4) = ARGV #: Array[String]
T.reveal_type(let3) # error: Revealed type: `T.nilable(String)`
T.reveal_type(let4) # error: Revealed type: `T::Array[String]`

let5, *let6 = (ARGV) #: Array[String]
T.reveal_type(let5) # error: Revealed type: `T.nilable(String)`
T.reveal_type(let6) # error: Revealed type: `T::Array[String]`

(let7, *let8 = ARGV) #: Array[String]
T.reveal_type(let7) # error: Revealed type: `T.untyped`
T.reveal_type(let8) # error: Revealed type: `T::Array[T.untyped]`
