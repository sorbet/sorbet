# typed: strict

module BarMethods
  extend T::Sig

  sig {returns(Project::Foo::Foo)}
  def build_foo
    # Construct an imported class.
    Project::Foo::Foo.new(10)
  end

  sig {returns(T::Boolean)}
  def check_bar
    # Call an imported method.
    Project::Foo.good_check_is_bar(Bar.new(5))
  end
end
