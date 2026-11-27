# typed: strict

class ConstantAnnotatedInRbi
  # The type annotations for these constants are in the `.rbi` file. Hash literals have no inferred
  # type, so without an annotation, a `T.let` would be requested here.
  MY_HASH = {
    "a" => "b",
  }.freeze

  UNCHECKED_HASH = {
    "c" => "d",
  }.freeze
end

T.reveal_type(ConstantAnnotatedInRbi::MY_HASH) # error: Revealed type: `T::Hash[String, String]`
T.reveal_type(ConstantAnnotatedInRbi::UNCHECKED_HASH) # error: Revealed type: `T::Hash[String, String]`
