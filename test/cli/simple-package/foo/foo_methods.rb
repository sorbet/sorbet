# frozen_string_literal: true
# typed: strict

module FooMethods
  extend T::Sig

  sig {returns(Project::Bar::Bar)}
  def self.build_bar
    # Construct an imported class.
    Project::Bar::Bar.new(10)
  end

  sig {returns(Foo)}
  def self.build_foo
    # Call an imported method.
    Project::Bar::BarMethods.build_foo
  end
end
