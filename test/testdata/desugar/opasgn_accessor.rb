# typed: true

class A
  extend T::Sig

  sig {returns(T.nilable(String))}
  attr_accessor :value
end

a = A.new

# Ensure that desugarings of op-assignment is behaving

a.value &&= "string"
a.value ||= "string"
