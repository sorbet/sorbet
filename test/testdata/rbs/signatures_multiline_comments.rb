# typed: strict
# enable-experimental-rbs-comments: true

#: -> [
#|   String, # name
#|   Integer # age
#| ]
def foo
  ["Alice", 30]
end
T.reveal_type(foo) # error: Revealed type: `T::Array[T.any(String, Integer)]`

#: ( # parameters
#|   # A comment-only continuation with UTF-8: café
#|   String value # input
#| ) -> String # output
def identity(value)
  value
end
T.reveal_type(identity("# literal")) # error: Revealed type: `String`

#: ( # preserve locations after comments
#|   MissingType # unknown type
#    ^^^^^^^^^^^ error: Unable to resolve constant `MissingType`
#| ) -> void
def missing_type(value); end

#: ( # parse errors must also retain their locations
#|   String
#|   -> void
#    ^^ error: Failed to parse RBS signature (unexpected token for function parameter name)
def invalid(value); end # error: The method `invalid` does not have a `sig`

#: ( # Hashes inside quoted literals are not comments
#|   "#" | :"#"
#    ^^^ error: RBS literal types are not supported
#          ^^^^ error: RBS literal types are not supported
#| ) -> void
def hash_literal(value); end
