# typed: true
# disable-fast-path: true

class Mutator
  extend T::Generic

  ModelType = type_template
end

class MyMutator < Mutator # error: must be re-declared
end

module AbstractRPCMethod
  extend T::Generic

  RPCInput = type_member
  RPCOutput = type_member
end

class MyRPCMethod1 # error-with-dupes: must be re-declared
  # ^ This autocorrect is weird, as it inserts the autocorrect here.
  #
  # As it turns out, that's fine--there's no requirement that the type member
  # definition must come after the ancestor definition (neither for it to be
  # correct according to the VM nor according to Sorbet)
  #
  # HOWEVER, at the moment our autocorrects are limited--all these autocorrects
  # get inserted at the same loc. As a defensive mechanism, we only apply the
  # first edit at a given location, which means that it requires applying
  # either two code actions or two rounds of --autocorrect to fix this error.
  # That seems ~fine.
  extend AbstractRPCMethod
end

class MyRPCMethod2 # error: must be re-declared
  extend T::Generic
  RPCInput = type_template
  extend AbstractRPCMethod
end

class NumericBox
  extend T::Generic

  Elem = type_member(:out) { {upper: Numeric} }
end

class IntegerBox < NumericBox # error: must be re-declared
  # ^ This autocorrect does not copy the upper bounds, but it should copy the
  # variance. (It would be neat if it could do both, but we don't have that
  # information resolved by the time we run this error.)
end
