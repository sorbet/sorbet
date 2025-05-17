# typed: strict
# enable-experimental-rbs-comments: true

# let

let1 = ARGV.first #: String
T.reveal_type(let1) # error: Revealed type: `String`

let2 = nil #: Integer?
T.reveal_type(let2) # error: Revealed type: `T.nilable(Integer)`

let3 =
    nil #: Integer?
T.reveal_type(let3) # error: Revealed type: `T.nilable(Integer)`

let4 = ARGV
      .first
      .strip #: String
T.reveal_type(let4) # error: Revealed type: `String`

let5 = ARGV.first #: String#some comment
T.reveal_type(let5) # error: Revealed type: `String`

let6 = ARGV.first #: String # some comment
T.reveal_type(let6) # error: Revealed type: `String`

let7 = ARGV.first#: String
T.reveal_type(let7) # error: Revealed type: `String`

let8 = ARGV.first#:String
T.reveal_type(let8) # error: Revealed type: `String`

let9 = (ARGV.first) #: String
T.reveal_type(let9) # error: Revealed type: `String`

(let10 = ARGV.first) #: String
T.reveal_type(let10) # error: Revealed type: `T.untyped`

LET1 = ARGV.first #: String
T.reveal_type(LET1) # error: Revealed type: `String`

LET2 = T.must("foo"[0]) #: String
T.reveal_type(LET2) # error: Revealed type: `String`

@let1 = ARGV.first #: String
T.reveal_type(@let1) # error: Revealed type: `String`

@let2 = ARGV.first || [] #: Array[String]
T.reveal_type(@let2) # error: Revealed type: `T::Array[String]`

@let3 = ARGV.first&.strip #: String?
T.reveal_type(@let3) # error: Revealed type: `T.nilable(String)`

@@let1 = 1 #: Integer
T.reveal_type(@@let1) # error: Revealed type: `Integer`

$let1 = ARGV.first #: String
T.reveal_type($let1) # error: Revealed type: `String`

# cast

cast1 = "#: Integer" #: String
T.reveal_type(cast1) # error: Revealed type: `String`

cast2 = ARGV.first #:as String
T.reveal_type(cast2) # error: Revealed type: `String`

cast3 = ARGV.first #: as String
T.reveal_type(cast3) # error: Revealed type: `String`

cast4 = ARGV.first #:      as      String
T.reveal_type(cast4) # error: Revealed type: `String`

cast5 = ARGV.first #: as String#some comment
T.reveal_type(cast5) # error: Revealed type: `String`

cast6 = ARGV.first #: as String # some comment
T.reveal_type(cast6) # error: Revealed type: `String`

cast7 = ARGV.first#:as String # some comment
T.reveal_type(cast7) # error: Revealed type: `String`

cast8 = "a" #: as untyped
  .random_method_name

# must

must_x = ARGV.first #:as String?

must1 = must_x #:as !nil
T.reveal_type(must1) # error: Revealed type: `String`

must2 = must_x #: as !nil
T.reveal_type(must2) # error: Revealed type: `String`

must3 = must_x #:      as      !nil
T.reveal_type(must3) # error: Revealed type: `String`

must4 = must_x #: as !nil #some comment
T.reveal_type(must4) # error: Revealed type: `String`

must5 = must_x#: as !nil # some comment
T.reveal_type(must5) # error: Revealed type: `String`

# unsafe

unsafe_x = ARGV.first #:as String?

unsafe1 = unsafe_x #: as untyped
T.reveal_type(unsafe1) # error: Revealed type: `T.untyped`

unsafe2 = unsafe_x #: as untyped
T.reveal_type(unsafe2) # error: Revealed type: `T.untyped`

unsafe3 = unsafe_x #:      as      untyped
T.reveal_type(unsafe3) # error: Revealed type: `T.untyped`
