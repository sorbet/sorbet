# typed: strict
# enable-experimental-rbs-comments: true

# let

let1 = "" #: String?
let1 &&= "foo" #: Integer # error: Argument does not have asserted type `Integer`
T.reveal_type(let1) # error: Revealed type: `T.nilable(Integer)`

let2 = "" #: String?
let3 = (let2 &&= "foo") #: Integer
                        #  ^^^^^^^ error: Argument does not have asserted type `Integer`
T.reveal_type(let2) # error: Revealed type: `T.nilable(String)`
T.reveal_type(let3) # error: Revealed type: `Integer`

let4 = "" #: String?
(let4 &&= "foo") #: Integer
                 #  ^^^^^^^ error: Argument does not have asserted type `Integer`
T.reveal_type(let4) # error: Revealed type: `T.nilable(String)`

# cast

cast1 = "" #: as String?
           #     ^^^^^^^ error: `T.cast` is useless because `String("")` is already a subtype of `T.nilable(String)`
cast1 &&= "foo" #: as Integer
T.reveal_type(cast1) # error: Revealed type: `T.nilable(Integer)`

# must

must1 = "" #: as !nil
#       ^^ error: `T.must` called on `String("")`, which is never `nil`
must1 &&= "foo" #: as !nil
#         ^^^^^ error: `T.must` called on `String("foo")`, which is never `nil`
T.reveal_type(must1) # error: Revealed type: `String("foo")`
