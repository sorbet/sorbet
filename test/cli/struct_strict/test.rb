# typed: strict

class Module
  include T::Sig
end

Bad = Struct.new(:foo) do
end

Okay = Struct.new(:foo) do
  sig {returns(T.nilable(Integer))}
  def foo; super; end

  sig {params(foo: T.nilable(Integer)).returns(T.nilable(Integer))}
  def foo=(foo); super; end
end
