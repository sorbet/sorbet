# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig
  sig {returns(Integer)}
  def sigfun
    85
  end

  # Only very limited forms of multiply-defined methods are supported in the
  # Sorbet compiler.  This is one of those cases, where we can immediately
  # redefine the method.
  def sigfun
    84
  end
end

# The signature for the redefined method should be picked up from sorbet-runtime.
p A.new.sigfun
s = T::Utils.signature_for_method(A.instance_method(:sigfun))
p s.nil?
p s&.method&.name
p s&.method_name
p s&.return_type&.name
