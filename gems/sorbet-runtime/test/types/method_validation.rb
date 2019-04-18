# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class MethodValidationTest < Critic::Unit::UnitTest
    before do
      @mod = Module.new do
        extend T::Sig
        # Make it public for testing only
        public_class_method :sig
      end
    end

    describe "declaration" do

      it "succeeds with untyped" do
        @mod.sig {params(bar: T.untyped).returns(T.untyped)}
        def @mod.foo(bar)
          :foo
        end
        @mod.foo(1)
      end

      it "raises an error when noreturn method returns" do
        @mod.sig {params(bar: String).returns(T.noreturn)}
        def @mod.foo(bar)
          :foo
        end
        assert_raises(TypeError) do
          @mod.foo("1")
        end
      end

      it "raises an error when method with noreturn argument method is called" do
        @mod.sig {params(bar: T.noreturn).returns(String)}
        def @mod.foo(bar)
          :foo
        end
        assert_raises(TypeError) do
          @mod.foo(1)
        end
      end

      it "raises an error if there are invalid parameter names in the declaration" do
        @mod.sig {params(bar: String).returns(Symbol)}
        def @mod.foo
          :foo
        end
        err = assert_raises(RuntimeError) do
          @mod.foo
        end
        assert_equal("The declaration for `foo` has extra parameter(s): bar", err.message)
      end

      it "raises an error if parameters are missing from the declaration" do
        @mod.sig {returns(Symbol)}
        def @mod.foo(bar)
          :foo
        end
        err = assert_raises(RuntimeError) do
          @mod.foo
        end
        assert_equal("The declaration for `foo` is missing parameter(s): bar", err.message)
      end

      it "raises an error if parameters are declared in the wrong order" do
        @mod.sig {params(bar: String, foo: String).returns(Symbol)}
        def @mod.foo(foo, bar)
          :foo
        end
        err = assert_raises(RuntimeError) do
          @mod.foo
        end
        assert_includes(
          err.message,
          "Parameter `bar` is declared out of order (declared as arg number 1, defined in the " \
          "method as arg number 2)."
        )
        assert_includes(err.message, "#{__FILE__}:#{__LINE__ - 11}")
      end

      it "raises an error when calling sig twice in a row" do
        @mod.sig {returns(Symbol)}
        err = assert_raises(RuntimeError) do
          @mod.sig {returns(Symbol)}
        end
        assert_equal(
          "You called sig twice without declaring a method inbetween",
          err.message
        )
      end

      it "raises an error when adding a method to a different module than the last declaration" do
        mod2 = Module.new do
					extend T::Sig
          sig {returns(String)}
          def foo; end
        end

        @mod.sig {returns(Symbol)}
        def mod2.bar; end
        err = assert_raises(RuntimeError) do
          mod2.bar
        end

        assert_equal(
          "A method (bar) is being added on a different class/module (#{mod2}) than the last call to " \
          "`sig` (#{@mod}). Make sure each call to `sig` is immediately " \
          "followed by a method definition on the same class/module.",
          err.message
        )
      end

      it "gives a helpful error if you order optional kwargs after required" do
        ex = assert_raises(RuntimeError) do
          mod = Module.new do
						extend T::Sig
            sig {params(a: Integer, b: Integer).returns(Integer)}
            def self.foo(a: 1, b:)
              a + b
            end
          end
          mod.foo
        end
        assert_match(/required keyword arguments must precede any optional keyword arguments/,
                     ex.message)
      end

      it "does not allocate much" do
        @mod.sig do
          params(
            x: Boolean,
            y: T::Array[Symbol],
            z: T::Hash[Symbol, String]
          )
          .returns(T::Array[Symbol])
        end
        def @mod.foo(x, y, z)
          y
        end
        TEST_DATA = {
          x: true,
          y: 50.times.map do |i|
            "foo_#{i}".to_sym
          end,
          z: 50.times.map do |i|
            ["bar_#{i}".to_sym, i.to_s]
          end.to_h,
        }

        Critic::Extensions::TypeExt.unpatch_types
        @mod.foo(TEST_DATA[:x], TEST_DATA[:y], TEST_DATA[:z]) # warmup, first run runs in mixed mode, when method is replaced but called in a weird way
        @mod.foo(TEST_DATA[:x], TEST_DATA[:y], TEST_DATA[:z]) # warmup, second run runs in real mode
        before = GC.stat(:total_allocated_objects)
        @mod.foo(TEST_DATA[:x], TEST_DATA[:y], TEST_DATA[:z])
        allocated = GC.stat(:total_allocated_objects) - before
        Critic::Extensions::TypeExt.patch_types
        if Gem::Version.new('2.6') <= Gem::Version.new(RUBY_VERSION)
          assert_equal(3, allocated)
        else
          assert_equal(4, allocated)
        end
        # see https://git.corp.stripe.com/stripe-internal/pay-server/pull/103670 for where 4 of those allocations come from
        # the others come from test harness in this test.
      end

      it "allocates little for simple sig" do
        @mod.sig do
          params(
            x: Boolean,
            y: Integer,
          )
            .returns(Integer)
        end
        def @mod.foo(x, y)
          y
        end

        @mod.foo(true, 1) # warmup, first run runs in mixed mode, when method is replaced but called in a weird way
        @mod.foo(true, 1) # warmup, second run runs in real mode
        before = GC.stat(:total_allocated_objects)
        @mod.foo(true, 1)
        allocated = GC.stat(:total_allocated_objects) - before
        assert_equal(2, allocated) # dmitry: for some reason, when run locally this numeber is 0, in CI it's 2. IDK why.
      end
    end

    describe "validation" do
      it "raises an error when the return value is the wrong type" do
        @mod.sig {returns(String)}
        def @mod.foo
          :foo
        end

        err = assert_raises(TypeError) do
          @mod.foo
        end

        lines = err.message.split("\n")
        assert_equal("Return value: Expected type String, got type Symbol with value :foo", lines[0])
        # Note that the paths here could be relative or absolute depending on how this test was invoked.
        assert_match(%r{\ACaller: .*test.*/types/method_validation.rb:#{__LINE__ - 6}\z}, lines[1])
        assert_match(%r{\ADefinition: .*test.*/types/method_validation.rb:#{__LINE__ - 12}\z}, lines[2])
        assert_empty(lines[3..-1])
      end

      it "raises an error when a param is the wrong type" do
        @mod.sig {params(bar: Integer).returns(String)}
        def @mod.foo(bar)
          :foo
        end

        err = assert_raises(TypeError) do
          @mod.foo(nil)
        end

        lines = err.message.split("\n")
        assert_equal("Parameter 'bar': Expected type Integer, got type NilClass", lines[0])
        # Note that the paths here could be relative or absolute depending on how this test was invoked.
        assert_match(%r{\ACaller: .*test.*/types/method_validation.rb:#{__LINE__ - 6}\z}, lines[1])
        assert_match(%r{\ADefinition: .*test.*/types/method_validation.rb:#{__LINE__ - 12}\z}, lines[2])
        assert_empty(lines[3..-1])
      end

      it "raises an error when the block is the wrong type" do
        @mod.sig {params(blk: Proc).returns(String)}
        def @mod.foo(&blk)
          :foo
        end

        err = assert_raises(TypeError) do
          @mod.foo
        end

        lines = err.message.split("\n")
        assert_equal("Block parameter 'blk': Expected type Proc, got type NilClass", lines[0])
        # Note that the paths here could be relative or absolute depending on how this test was invoked.
        assert_match(%r{\ACaller: .*test.*/types/method_validation.rb:#{__LINE__ - 6}\z}, lines[1])
        assert_match(%r{\ADefinition: .*test.*/types/method_validation.rb:#{__LINE__ - 12}\z}, lines[2])
        assert_empty(lines[3..-1])
      end

      it "raises an error when the bind is the wrong type" do
        @mod.sig {bind(Integer).returns(Integer)}
        def @mod.foo
          self + 2
        end

        err = assert_raises(TypeError) do
          @mod.foo
        end

        lines = err.message.split("\n")
        assert_match(/Bind: Expected type Integer, got type Module with value #<Module:0x/, lines[0])
        # Note that the paths here could be relative or absolute depending on how this test was invoked.
        assert_match(%r{\ACaller: .*test.*/types/method_validation.rb:#{__LINE__ - 6}\z}, lines[1])
        assert_match(%r{\ADefinition: .*test.*/types/method_validation.rb:#{__LINE__ - 12}\z}, lines[2])
        assert_empty(lines[3..-1])
      end

      it "accepts T.proc" do
        @mod.sig {params(blk: T.proc.params(i: Integer).returns(Integer)).returns(Integer)}
        def @mod.foo(&blk)
          blk.call(4)
        end

        @mod.foo {|i| i * i}
      end

      it "allows procs to bind" do
        @mod.sig {params(blk: T.proc.bind(Integer).returns(Integer)).returns(Integer)}
        def @mod.foo(&blk)
          3.instance_eval(&blk)
        end

        assert_equal(7, @mod.foo {self + 4})
      end

      it "rejects T::Utils::RuntimeProfiled" do
        @mod.sig {returns(T::Utils::RuntimeProfiled)}
        def @mod.foo; end

        err = assert_raises(TypeError) do
          @mod.foo
        end

        assert_match("Expected type T::Utils::RuntimeProfiled, got type NilClass", err.message)
      end

      it "gets the locations right with the second call" do
        @mod.sig {returns(String)}
        def @mod.foo
          :foo
        end

        assert_raises(TypeError) do
          @mod.foo
        end
        err = assert_raises(TypeError) do
          @mod.foo
        end

        lines = err.message.split("\n")
        # Note that the paths here could be relative or absolute depending on how this test was invoked.
        assert_match(%r{\ACaller: .*test.*/types/method_validation.rb:#{__LINE__ - 5}\z}, lines[1])
        assert_empty(lines[3..-1])
      end

      describe "instance methods" do
        it "raises an error when the return value is the wrong type " do
          klass = Class.new do
						extend T::Sig
            sig {returns(String)}
            def foo
              :foo
            end
          end

          err = assert_raises(TypeError) do
            klass.new.foo
          end
          assert_match(/\AReturn value: Expected type String, got type Symbol/, err.message)
        end
      end

      describe "distinguishing between args and kwargs" do
        before do
          @mod.sig do
            params(
              req_str: String,
              opt_hash: Hash,
              kwopt_bool: Boolean,
            )
            .returns(Symbol)
          end
          def @mod.foo(req_str, opt_hash={}, kwopt_bool: false)
            :foo
          end
        end

        it "correctly validates a hash passed as the required arg (not treating it as kwargs)" do
          err = assert_raises(TypeError) do
            @mod.foo({kwopt_bool: true})
          end
          assert_match(/\AParameter 'req_str': Expected type String, got type Hash/, err.message)
        end

        it "correctly validates a hash passed as the required arg using implied-hash syntax (not treating it as kwargs)" do
          err = assert_raises(TypeError) do
            @mod.foo(kwopt_bool: true)
          end
          assert_match(/\AParameter 'req_str': Expected type String, got type Hash/, err.message)
        end

        it "correctly validates a hash passed as the optional arg (treating it as kwargs)" do
          err = assert_raises(TypeError) do
            @mod.foo("foo", {kwopt_bool: 42})
          end
          assert_match(/\AParameter 'kwopt_bool': Expected type Boolean, got type Integer/, err.message)
        end

        it "correctly validates a hash passed as the optional arg using implied-hash syntax (treating it as kwargs)" do
          err = assert_raises(TypeError) do
            @mod.foo("foo", kwopt_bool: 42)
          end
          assert_match(/\AParameter 'kwopt_bool': Expected type Boolean, got type Integer/, err.message)
        end
      end

      it 'raises a soft_assertion when .soft is used with a notify' do
        @mod.sig {returns(Symbol).soft(notify: 'hello')}
        def @mod.foo
          1
        end

        Opus::Error.expects(:soft).with(
          regexp_matches(/TypeError: Return value: Expected type Symbol, got type Integer with value 1\nCaller: .+\d\nDefinition: .+\d/),
          notify: 'hello'
        )
        @mod.foo
      end

      it 'logs with generated' do
        @mod.sig {generated.returns(Symbol)}
        def @mod.foo
          1
        end

        Opus::Log.stubs(:info).once.with do |message|
          assert_equal('SIG-CHECK-FAILED', message)
          true
        end
        @mod.foo
      end

      it 'logs with generated, but only once' do
        @mod.sig {generated.returns(Symbol)}
        def @mod.foo
          1
        end

        Opus::Log.stubs(:info).once.with do |message|
          assert_equal('SIG-CHECK-FAILED', message)
          true
        end
        @mod.foo
        @mod.foo
      end

      it 'does not throw if malformed but with .generated' do
        @mod.sig {generated.returns(Integer)}
        def @mod.foo(foo)
          1
        end

        Opus::Log.stubs(:info).once.with do |message|
          assert_equal('SIG-DECLARE-FAILED', message)
          true
        end
        @mod.foo(2)
        @mod.foo(2)
      end

      it 'does not throw if parent is .generated' do
        parent = Class.new do
					extend T::Sig
          sig {generated.returns(Integer)}
          def foo
            1
          end
        end

        child = Class.new(parent) do
					extend T::Sig
          sig {returns(String)}
          def foo
            "1"
          end
        end

        Opus::Log.stubs(:info).once.with do |message|
          assert_equal('SIG-DECLARE-FAILED', message)
          true
        end
        child.new.foo
      end

      it 'logs nicely for Enumerables' do
        @mod.sig {generated.returns(Symbol)}
        def @mod.foo
          [[{a: 1}, 2], "3", 4..5]
        end

        Opus::Log.stubs(:info).once.with do |message, *args|
          assert_equal('SIG-CHECK-FAILED', message)
          assert_equal('T::Array[T.any(String, T::Array[T.any(Integer, T::Hash[Symbol, Integer])], T::Range[Integer])]', args[0][:got])
          true
        end
        @mod.foo
      end

      it 'handles splats' do
        @mod.sig {params(args: Integer).void}
        def @mod.foo(*args); end

        @mod.foo(2, 3, 4)

        err = assert_raises(TypeError) do
          @mod.foo(2, 'hi')
        end
        assert_match(/\AParameter 'args': Expected type Integer, got type String with value "hi"/, err.message)
      end

      it 'handles splats with real params' do
        @mod.sig {params(bar: String, args: Integer).void}
        def @mod.foo(bar, *args); end

        @mod.foo('hi', 2)

        err = assert_raises(TypeError) do
          @mod.foo(2, 'hi')
        end
        assert_match(/\AParameter 'bar': Expected type String, got type Integer with value 2/, err.message)
      end

      it 'handles keyrest' do
        @mod.sig {params(opts: Integer).void}
        def @mod.foo(**opts); end

        @mod.foo(a: 2, b: 3, c: 4)

        err = assert_raises(TypeError) do
          @mod.foo(a: 2, b: 3, bar: 'hi', c: 4)
        end
        assert_match(/\AParameter 'bar': Expected type Integer, got type String with value "hi"/, err.message)
      end

      it 'handles keyrest with other params' do
        @mod.sig {params(bar: String, opts: Integer).void}
        def @mod.foo(bar:, **opts); end

        @mod.foo(a: 2, b: 3, bar: 'hi', c: 4)

        err = assert_raises(TypeError) do
          @mod.foo(a: 2, b: 3, bar: 'hi', c: 'bye')
        end
        assert_match(/\AParameter 'c': Expected type Integer, got type String with value "bye"/, err.message)
      end
    end

    # These tests should behave identically with and without a declaration
    [false, true].each do |using_declaration|
      describe "handling missing args (using_declaration=#{using_declaration}" do
        before do
          if using_declaration
            @mod.sig do
              params(
                req_hash: Hash,
                opt_array: Array,
                kwreq_int: Integer,
                kwopt_bool: Boolean,
                blk: T.nilable(Proc),
              )
              .returns([String, Float])
            end
          end
          def @mod.foo(req_hash, opt_array=[], kwreq_int:, kwopt_bool: false, &blk)
            ["foo", 42.0]
          end
        end

        it "runs without error with correct types" do
          @mod.foo({}, [], kwreq_int: 1, kwopt_bool: true) {}
        end

        it "fails if a required arg is missing" do
          err = assert_raises(ArgumentError) do
            @mod.foo
          end
          assert_match(/wrong number of arguments \(given 0, expected 1..2/, err.message)
        end

        it "fails if a required kwarg is missing" do
          err = assert_raises(ArgumentError) do
            @mod.foo({})
          end
          assert_equal("missing keyword: kwreq_int", err.message)
        end
      end
    end

    class A
      def foo; end
    end
  end
end
