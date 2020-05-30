# typed: strict

class Bar
  extend T::Sig
  
  sig {returns(Project::Foo::Foo)}
  def self.build_foo
    # Construct an imported class.
    Project::Foo::Foo.new(10)
  end

  sig {returns(Bar)}
  def self.build_bar
    # Call an imported method.
    Project::Foo::Foo.build_bar
  end

  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end
end
