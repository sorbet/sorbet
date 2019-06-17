# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class ConfigurationTest < Critic::Unit::UnitTest
    before do
      @mod = Module.new do
        extend T::Sig
        # Make it public for testing only public_class_method :sig
      end
    end

    class CustomReceiver
      def self.receive(*); end
    end

    describe 'inline_type_error_handler' do
      describe 'when in default state' do
        it 'T.must raises an error' do
          assert_raises(TypeError) do
            T.must(nil)
          end
        end

        it 'T.let raises an error' do
          assert_raises(TypeError) do
            T.let(1, String)
          end
        end
      end

      describe 'when overridden' do
        before do
          T::Configuration.inline_type_error_handler = lambda do |*args|
            CustomReceiver.receive(*args)
          end
        end

        after do
          T::Configuration.inline_type_error_handler = nil
        end

        it 'handles a T.must error' do
          CustomReceiver.expects(:receive).once.with do |error|
            error.is_a?(TypeError)
          end
          assert_nil(T.must(nil))
        end

        it 'handles a T.let error' do
          CustomReceiver.expects(:receive).once.with do |error|
            error.is_a?(TypeError)
          end
          assert_equal(1, T.let(1, String))
        end
      end
    end

    describe 'sig_builder_error_handler' do
      describe 'when in default state' do
        it 'raises an error' do
          @mod.sig {returns(Symbol).void}
          def @mod.foo
            :bar
          end
          ex = assert_raises(ArgumentError) do
            @mod.foo
          end
          assert_includes(
            ex.message,
            "You can't call .void after calling .returns."
          )
        end
      end

      describe 'when overridden' do
        before do
          T::Configuration.sig_builder_error_handler = lambda do |*args|
            CustomReceiver.receive(*args)
          end
        end

        after do
          T::Configuration.sig_builder_error_handler = nil
        end

        it 'handles a sig builder error' do
          CustomReceiver.expects(:receive).once.with do |error, location|
            error.message == "You can't call .void after calling .returns." &&
              error.is_a?(T::Private::Methods::DeclBuilder::BuilderError) &&
              location.is_a?(Thread::Backtrace::Location)
          end
          @mod.sig {returns(Symbol).void}
          def @mod.foo
            :bar
          end
          assert_equal(:bar, @mod.foo)
        end
      end
    end

    describe 'sig_validation_error_handler' do
      describe 'when in default state' do
        it 'raises an error' do
          @mod.sig {override.returns(Symbol)}
          def @mod.foo
            :bar
          end
          ex = assert_raises(RuntimeError) do
            @mod.foo
          end
          assert_includes(
            ex.message,
            "You marked `foo` as .override, but that method doesn't already exist"
          )
        end
      end

      describe 'when overridden' do
        before do
          T::Configuration.sig_validation_error_handler = lambda do |*args|
            CustomReceiver.receive(*args)
          end
        end

        after do
          T::Configuration.sig_validation_error_handler = nil
        end

        it 'handles a sig build error' do
          CustomReceiver.expects(:receive).once.with do |error, opts|
            error.message.include?("You marked `foo` as .override, but that method doesn't already exist") &&
              error.is_a?(RuntimeError) &&
              opts.is_a?(Hash) &&
              opts[:method].is_a?(UnboundMethod) &&
              opts[:declaration].is_a?(T::Private::Methods::Declaration) &&
              opts[:signature].is_a?(T::Private::Methods::Signature)
          end

          @mod.sig {override.returns(Symbol)}
          def @mod.foo
            :bar
          end
          assert_equal(:bar, @mod.foo)
        end
      end
    end

    describe 'call_validation_error_handler' do
      describe 'when in default state' do
        it 'raises an error' do
          @mod.sig {params(a: String).returns(Symbol)}
          def @mod.foo(a)
            :bar
          end
          ex = assert_raises(TypeError) do
            @mod.foo(1)
          end
          assert_includes(
            ex.message,
            "Parameter 'a': Expected type String, got type Integer with value 1"
          )
        end
      end

      describe 'when overridden' do
        before do
          T::Configuration.call_validation_error_handler = lambda do |*args|
            CustomReceiver.receive(*args)
          end
        end

        after do
          T::Configuration.call_validation_error_handler = nil
        end

        it 'handles a sig error' do
          CustomReceiver.expects(:receive).once.with do |signature, opts|
            signature.is_a?(T::Private::Methods::Signature) &&
              opts.is_a?(Hash) &&
              opts[:name] == :a &&
              opts[:kind] == 'Parameter' &&
              opts[:type].name == 'String' &&
              opts[:value] == 1 &&
              opts[:location].is_a?(Thread::Backtrace::Location) &&
              opts[:message].include?("Expected type String, got type Integer with value 1")
          end
          @mod.sig {params(a: String).returns(Symbol)}
          def @mod.foo(a)
            :bar
          end
          assert_equal(:bar, @mod.foo(1))
        end
      end
    end

    describe 'scalar_types' do
      describe 'when overridden' do
        before do
          T::Configuration.scalar_types = ['foo']
        end

        after do
          T::Configuration.scalar_types = nil
        end

        it 'contains the correct values' do
          assert_equal(T::Configuration.scalar_types, Set.new(['foo']))
        end

        it 'requires string values' do
          ex = assert_raises(ArgumentError) do
            T::Configuration.scalar_types = [1, 2, 3]
          end
          assert_includes(ex.message, "Provided values must all be class name strings.")
        end
      end
    end
  end
end
