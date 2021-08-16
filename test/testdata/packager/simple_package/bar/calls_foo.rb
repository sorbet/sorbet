# frozen_string_literal: true
# typed: strict

module Project::Bar::CallsFoo
  extend T::Sig

  sig {returns(Project::Foo::Foo)}
  def self.build_foo
    # Construct an imported class.
    Project::Foo::Foo.new(10)
  end

  sig {returns(Project::Bar::Bar)}
  def self.build_bar
    # Call an imported method.
    Project::Foo::CallsBar.build_bar
  end
end
