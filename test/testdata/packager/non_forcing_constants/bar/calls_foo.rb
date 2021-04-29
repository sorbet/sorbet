# frozen_string_literal: true
# typed: strict

module Project::Bar::CallsFoo
  extend T::Sig

  sig {returns(Project::Foo::Foo)}
  def build_foo
    # Construct an imported class.
    Project::Foo::Foo.new(10)
  end

  sig {returns(T::Boolean)}
  def check_bar
    # Call an imported method.
    Project::Foo::FooNonForcing.good_check_is_bar(Project::Bar::Bar.new(5))
  end
end
