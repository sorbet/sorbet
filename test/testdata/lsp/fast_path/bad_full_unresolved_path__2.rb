# typed: strict

class A
  extend T::Sig

  ATLP = Namespace::AliasToLongProto

  sig { returns(ATLP::DoesNotExist2) } # error: Unable to resolve constant
  def example = raise
end
