# typed: strict
# frozen_string_literal: true

# stratum: 2

class Parent::MyClass # error: File belongs to package `Test::Parent` but defines a constant that does not match this namespace
  include Test::Parent::TestMixin
end
