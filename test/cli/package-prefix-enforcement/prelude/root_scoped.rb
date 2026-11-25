# typed: strict

module ::RootScopedFromPrelude
  extend T::Sig

  CONST = 1

  sig {void}
  def self.behavior
  end

  class Nested
  end
end

::RootScopedAssignmentFromPrelude = 1
