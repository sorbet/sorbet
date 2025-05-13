# typed: strict
# enable-experimental-rbs-comments: true

# let

let1 = 42 #: Integer
let1 += "" #: String # error: Expected `Integer` but found `String` for argument `arg0`
T.reveal_type(let1) # error: Revealed type: `Integer`

let2 = 42 #: Integer
let2 += ("foo") #: String
                #  ^^^^^^ error: Expected `Integer` but found `String` for argument `arg0`
T.reveal_type(let2) # error: Revealed type: `Integer`

let3 = 42 #: Integer
(let3 += "foo") #: String
#        ^^^^^ error: Expected `Integer` but found `String("foo")` for argument `arg0`
#                  ^^^^^^ error: Argument does not have asserted type `String`
T.reveal_type(let3) # error: Revealed type: `Integer`
