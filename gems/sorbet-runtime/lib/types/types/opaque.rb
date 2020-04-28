# frozen_string_literal: true
# typed: true

module T::Types
  class Opaque
    extend T::Sig
    extend T::Generic

    ConcreteType = type_template

    sig(:final) {params(obj: ConcreteType).returns(T.attached_class)}
    def self.let(obj)
      T.unsafe(obj)
    end

    sig(:final) {returns(Class)}
    def self.concrete_type
      self.const_get(:ConcreteType, false).fixed
    end

    # There should never be instances of Opaque types
    private_class_method :new
  end
end
