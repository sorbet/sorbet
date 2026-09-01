# typed: strict

module MustTestFoo
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.void }
  it "foo" do
  end
end

class A
  include MustTestFoo

  it "foo" do
  end
end
