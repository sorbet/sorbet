# typed: strict

module FooMethods
  extend T::Sig

  sig {returns(Project::Bar::Bar)}
  def build_bar
    # Construct an imported class.
    Project::Bar::Bar.new(10)
  end

  sig {returns(Foo)}
  def build_foo
    # Call an imported method.
    Project::Bar.build_foo
  end
end
