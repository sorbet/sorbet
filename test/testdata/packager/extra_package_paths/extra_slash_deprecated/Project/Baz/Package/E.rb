# frozen_string_literal: true

# typed: strict

module Project::Baz::Package
  class E
    extend T::Sig

    sig { void }
    def self.e; end
  end
end

