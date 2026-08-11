# typed: true

# Verify that String.dedup is defined in core RBIs with the correct signature.

T.reveal_type(String.dedup("foo")) # error: Revealed type: `String`

String.dedup("foo")
String.dedup(1) # error: Expected `String` but found `Integer(1)` for argument `string`
String.dedup()  # error: Not enough arguments
String.dedup("foo", "bar") # error: Too many arguments
