# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig
  sig {returns(Integer)}
  def self.sigfun
    85
  end

  sig {returns(String)}
  def sigfun
    "me!"
  end
end

p A.sigfun
s = T::Utils.signature_for_method(A.singleton_method(:sigfun))
p s.nil?
p s.method.name
p s.method_name
p s.return_type.name

p A.new.sigfun
s = T::Utils.signature_for_method(A.instance_method(:sigfun))
p s.nil?
p s.method.name
p s.method_name
p s.return_type.name
