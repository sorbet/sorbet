# typed: true
class FooBar
  extend T::Sig

  sig {void}
  def self.foo
    "foo"
  end

  sig {void}
  def self.bar
    "foo"
  end

  private_class_method([:foo, :bar])
end

FooBar.foo # error: Non-private call to private method `foo`
FooBar.bar # error: Non-private call to private method `bar`
