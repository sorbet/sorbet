# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::EdgeCasesTest < Critic::Unit::UnitTest
  it 'can type ==' do
    klass = Class.new do
      extend T::Helpers
      sig {override.params(other: T.self_type).returns(Boolean)}
      def ==(other)
        true
      end
    end
    assert_equal(klass.new, klass.new)
  end

  it 'handles aliased methods' do
    klass = Class.new do
      extend T::Helpers
      sig {returns(Symbol)}
      def foo
        :foo
      end
      alias_method :bar, :foo
    end
    assert_equal(:foo, klass.new.foo)
    assert_equal(:foo, klass.new.bar)
  end

  it 'works for any_instance' do
    klass = Class.new do
      extend T::Helpers
      def foo
        raise "bad"
      end

      sig {returns(Symbol)}
      def bar
        raise "bad"
      end
    end

    klass.any_instance.stubs(:foo)
    klass.new.foo

    klass.any_instance.stubs(:bar).returns(:bar)
    klass.new.bar
  end

  it 'works for calls_original' do
    klass = Class.new do
      extend T::Helpers
      sig {returns(Symbol)}
      def self.foo
        :foo
      end
    end

    # klass.stubs(:foo).calls_original # TODO not supported by Mocha
    assert_equal(:foo, klass.foo)
  end

  it 'works for stubbed superclasses with type' do
    parent = Class.new do
      extend T::Helpers
      sig {overridable.returns(Symbol)}
      def self.foo
        :parent
      end
    end
    child = Class.new(parent) do
      extend T::Helpers
      sig {override.returns(Symbol)}
      def self.foo
        :child
      end
    end
    parent.stubs(:foo)
    child.stubs(:foo).returns(:child_stub)
    assert_equal(:child_stub, child.foo)
  end

  it 'works for stubbed superclasses without type' do
    parent = Class.new do
      def self.foo
        :parent
      end
    end
    child = Class.new(parent) do
      extend T::Helpers
      sig {override.returns(Symbol)}
      def self.foo
        :child
      end
    end
    parent.stubs(:foo)
    child.stubs(:foo).returns(:child_stub)
    assert_equal(:child_stub, child.foo)
  end

  it 'allows private abstract methods' do
    klass = Class.new do
      extend T::Helpers
      abstract!

      sig {abstract.void}
      private def foo; end
    end
    T::Private::Abstract::Validate.validate_abstract_module(klass)
  end

  it 'handles class scope change when already hooked' do
    klass = Class.new do
      extend T::Helpers
      sig {returns(Symbol)}
      def foo
        :foo
      end

      class << self
        extend T::Helpers
        sig {returns(Symbol)}
        def foo
          :foo
        end
      end
    end
    assert_equal(:foo, klass.foo)
    assert_equal(:foo, klass.new.foo)
  end

  it 'handles class scope change when not already hooked' do
    klass = Class.new do
      T::Hooks.install(self)
      class << self
        extend T::Helpers
        sig {returns(Symbol)}
        def foo
          :foo
        end
      end
    end
    assert_equal(:foo, klass.foo)
  end

  it 'keeps raising for bad sigs' do
    klass = Class.new do
      extend T::Helpers
      sig {raise "foo"}
      def foo; end
    end
    instance = klass.new

    2.times do
      e = assert_raises {instance.foo}
      assert_equal("foo", e.message)
    end
  end

  it 'fails for sigs that fail then pass' do
    counter = 0
    klass = Class.new do
      extend T::Helpers
      sig do
        counter += 1
        raise "foo" if counter == 1
        void
      end
      def foo; end
    end
    instance = klass.new

    e = assert_raises {instance.foo}
    assert_equal("foo", e.message)
    e = assert_raises {instance.foo}
    assert_match(/A previous invocation of #<UnboundMethod: /, e.message)
  end

end
