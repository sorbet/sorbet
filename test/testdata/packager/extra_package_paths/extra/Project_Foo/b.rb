# frozen_string_literal: true

# typed: strict

class Project::Foo::B
  extend T::Sig

  sig { void }
  def self.b; end
end
