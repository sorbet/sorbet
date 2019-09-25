# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class AttachedClassTest < Critic::Unit::UnitTest
    it 'can type self methods' do

      class Base
        extend T::Sig

        sig {returns(AttachedClass)}
        def self.make
          new
        end
      end

      class A < Base; end

      assert_equal(Base.make.class, Base)
      assert_equal(A.make.class, A)
    end
  end
end
