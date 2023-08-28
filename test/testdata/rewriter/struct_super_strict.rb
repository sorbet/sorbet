# typed: strict
# highlight-untyped-values: true
class Module; include T::Sig; end

WithoutAnyOverride = Struct.new(:foo) do
  # We did not always generate `T.untyped` signatures for Struct members, which
  # prevented using `Struct.new` in `# typed: strict` files unless you went and
  # defined an override for every method.
end

ExplicitOverride = Struct.new(:foo) do
  sig {returns(String)}
  def foo
    super
  # ^^^^^ untyped: Value returned from method is `T.untyped`
  end

  sig {params(foo: String).returns(String)}
  def foo=(foo);
    super
  # ^^^^^ untyped: Value returned from method is `T.untyped`
  end
end
