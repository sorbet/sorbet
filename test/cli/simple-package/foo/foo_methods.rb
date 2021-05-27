# frozen_string_literal: true
# typed: strict

module Project::Foo
  module FooMethods
    extend T::Sig

    sig {returns(Project::Bar::BarClass)}
    def self.build_bar
      # Construct an imported class.
      Project::Bar::BarClass.new(10)
    end

    sig {returns(FooClass)}
    def self.build_foo
      # Call an imported method.
      Project::Bar::BarMethods.build_foo
    end
  end
end
