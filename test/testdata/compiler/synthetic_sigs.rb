# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  extend T::Sig

  # Sorbet will generate a `sig` for the synthetic declaration of this function,
  # but the compiler will not emit the actual definition -- that work will be
  # done by sorbet-runtime, and therefore we should mimic the behavior of
  # sorbet-runtime here.
  prop :foo, String
end

s = T::Utils.signature_for_method(A.instance_method(:foo))
p s.nil?
p s&.method&.name
p s&.method_name
p s&.return_type&.name

