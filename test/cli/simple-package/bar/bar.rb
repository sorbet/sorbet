# frozen_string_literal: true

# typed: strict

module Project::Bar
  class BarClass
    extend T::Sig
    sig { params(value: Integer).void }
    def initialize(value)
      @value = T.let(value, Integer)
    end
  end

  class UnexportedClass; end
end
