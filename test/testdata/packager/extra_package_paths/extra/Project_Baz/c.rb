# frozen_string_literal: true

# typed: strict

class Project::Baz::C
  extend T::Sig

  sig { void }
  def self.c; end
end
