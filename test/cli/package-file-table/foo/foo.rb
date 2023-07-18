# frozen_string_literal: true

# typed: strict

module Project::Foo
  class FooClass
    extend T::Sig
    sig { params(value: Integer).void }
    def initialize(value)
      @value = T.let(value, Integer)
    end

    Project::Bar::BardClass
    Project::Bar::UnexportedClass
  end
end
