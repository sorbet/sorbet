# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig
  sig {returns(Integer)}
  def sigfun
    85
  end
end

p A.new.sigfun
s = T::Utils.signature_for_method(A.instance_method(:sigfun))
p s.nil?
p s.method.name
p s.method_name
p s.return_type.name
