# typed: strict

module MustTestFoo
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.void } # error: Unused type annotation. No method def before next annotation
  it "foo" do
  end
end

class A
  include MustTestFoo

  it "foo" do
  end
end
