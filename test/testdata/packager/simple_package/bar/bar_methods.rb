# typed: strict

module BarMethods
  extend T::Sig

  sig {returns(Project::Foo::Foo)}
  def build_foo
    # Construct an imported class.
    Project::Foo::Foo.new(10)
  end

  sig {returns(Bar)}
  def build_bar
    # Call an imported method.
    Project::Foo.build_bar
  end
end
