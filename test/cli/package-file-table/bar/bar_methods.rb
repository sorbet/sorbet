# frozen_string_literal: true
# typed: strict

module Project::Bar
  module BarMethods
    extend T::Sig

    sig {returns(Project::Foo::FooClass)}
    def self.build_foo
      # Construct an imported class.
      Project::Foo::FooClass.new(10)
    end

    sig {returns(BarClass)}
    def self.build_bar
      # Call an imported method.
      Project::Foo::FooMethods.build_bar
    end
  end
end
