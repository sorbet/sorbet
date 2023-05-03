# typed: true

class Super
  extend T::Sig
  sig {params(x: Integer).returns(String)}
  def parent_method(x)
    x.to_s
  end
end

class A < Super
  alias_method(:bar, :parent_method)
end

A.new.parent_method(1)
A.new.bar(1)
