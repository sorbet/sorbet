# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class OpaqueTest < Critic::Unit::UnitTest
    extend T::Sig

    class OpaqueString < T::Types::Opaque
      ConcreteType = type_template(fixed: String)
    end

    describe 'coercion' do
      it 'allows a string' do
        assert(T::Utils.coerce(OpaqueString).valid?(""))
      end

      it 'does not allow an integer' do
        refute(T::Utils.coerce(OpaqueString).valid?(0))
      end
    end

    sig {params(x: OpaqueString).void}
    def takes_opaque_type(x); end

    sig {params(x: String).void}
    def takes_concrete_type(x); end

    describe 'method validation' do
      it 'allows either concrete or opaque type' do
        takes_opaque_type(OpaqueString.let(""))
        takes_opaque_type("")
        takes_concrete_type(OpaqueString.let(""))
        takes_concrete_type("")
      end
    end
  end
end
