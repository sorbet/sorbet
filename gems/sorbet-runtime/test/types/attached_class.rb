# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class AttachedClassTest < Critic::Unit::UnitTest
    it 'can type self methods (experimental)' do

      class Base
        extend T::Sig

        sig {returns(T.attached_class)}
        def self.make
          new
        end
      end

      class A < Base; end

      assert_equal(Base.make.class, Base)
      assert_equal(A.make.class, A)
    end

    it 'can type self methods' do

      class Base
        extend T::Sig

        sig {returns(T.attached_class)}
        def self.make
          new
        end
      end

      class A < Base; end

      assert_equal(Base.make.class, Base)
      assert_equal(A.make.class, A)
    end

    it 'can type self methods that use self.new' do

      class Base
        extend T::Sig

        sig {returns(T.attached_class)}
        def self.make
          self.new
        end
      end

      class Child < Base; end

      assert_equal(Base.make.class, Base)
      assert_equal(Child.make.class, Child)
    end

    it 'does not throw when the returned value is bad' do

      class Base
        extend T::Sig

        sig {returns(T.attached_class)}
        def self.make
          10
        end
      end

      assert_equal(Base.make.class, Integer)
    end

    it 'can print its own name' do
      class Base
        extend T::Sig

        sig {returns(T.attached_class)}
        def self.make
          Base.new
        end
      end

      meth = Base.method(:make)
      name = T::Private::Methods.signature_for_method(meth).return_type.to_s
      assert_equal("T.attached_class", name)
    end
  end
end
