# frozen_string_literal: true

# typed: strict

module Project::Baz::Package
  class C
    extend T::Sig

    sig { void }
    def self.c; end
  end
end
