# typed: strict

class Foo
  extend T::Sig

  sig {returns(Project::Bar::Bar)}
  def self.build_bar
    # Construct an imported class.
    Project::Bar::Bar.new(10)
  end

  sig {returns(Foo)}
  def self.build_foo
    # Call an imported method.
    Project::Bar::Bar.build_foo
  end

  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end
end
