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

    private def check_alloc_counts
      @check_alloc_counts = Gem::Version.new(RUBY_VERSION) < Gem::Version.new('3.0')
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
          "You called sig twice without declaring a method in between",
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

      it "does not allocate too much for complex sig" do
        @mod.sig do
          params(
            x: String,
            y: T::Array[Symbol],
            z: T::Hash[Symbol, String]
          )
          .returns(T::Array[Symbol])
        end
        def @mod.foo(x, y, z:)
          y
        end
        test_data = {
          x: "foo",
          y: Array.new(50) do |i|
            "foo_#{i}".to_sym
          end,
          z: Array.new(50) do |i|
            ["bar_#{i}".to_sym, i.to_s]
          end.to_h,
        }.freeze

        Critic::Extensions::TypeExt.unpatch_types
        @mod.foo(test_data[:x], test_data[:y], z: test_data[:z]) # warmup, first run runs in mixed mode, when method is replaced but called in a weird way
        @mod.foo(test_data[:x], test_data[:y], z: test_data[:z]) # warmup, second run runs in real mode
        before = GC.stat(:total_allocated_objects)
        @mod.foo(test_data[:x], test_data[:y], z: test_data[:z])
        allocated = GC.stat(:total_allocated_objects) - before
        Critic::Extensions::TypeExt.patch_types
        if check_alloc_counts
          if Gem::Version.new('2.6') <= Gem::Version.new(RUBY_VERSION)
            assert_equal(5, allocated)
          else
            assert_equal(6, allocated)
          end
        end
        # see https://git.corp.stripe.com/stripe-internal/pay-server/pull/103670 for where 4 of those allocations come from
        # the others come from test harness in this test.
      end

      it "allocates little for medium-complexity sig" do
        @mod.sig do
          params(
            x: String,
            y: T::Array[Symbol],
            z: T::Hash[Symbol, String]
          )
          .returns(T::Array[Symbol])
        end
        def @mod.foo(x, y, z)
          y
        end
        test_data = {
          x: "foo",
          y: Array.new(50) do |i|
            "foo_#{i}".to_sym
          end,
          z: Array.new(50) do |i|
            ["bar_#{i}".to_sym, i.to_s]
          end.to_h,
        }.freeze

        Critic::Extensions::TypeExt.unpatch_types
        @mod.foo(test_data[:x], test_data[:y], test_data[:z]) # warmup, first run runs in mixed mode, when method is replaced but called in a weird way
        @mod.foo(test_data[:x], test_data[:y], test_data[:z]) # warmup, second run runs in real mode
        before = GC.stat(:total_allocated_objects)
        @mod.foo(test_data[:x], test_data[:y], test_data[:z])
        allocated = GC.stat(:total_allocated_objects) - before
        Critic::Extensions::TypeExt.patch_types
        if check_alloc_counts
          expected_allocations = T::Configuration::AT_LEAST_RUBY_2_7 ? 1 : 2
          assert_equal(expected_allocations, allocated)
        end
      end

      it "allocates little for simple sig" do
        @mod.sig do
          params(
            x: String,
            y: Integer,
          )
          .returns(Integer)
        end
        def @mod.foo(x, y)
          y
        end

        @mod.foo("foo", 1) # warmup, first run runs in mixed mode, when method is replaced but called in a weird way
        @mod.foo("foo", 1) # warmup, second run runs in real mode
        before = GC.stat(:total_allocated_objects)
        @mod.foo("foo", 1)
        allocated = GC.stat(:total_allocated_objects) - before

        if check_alloc_counts
          expected_allocations = T::Configuration::AT_LEAST_RUBY_2_7 ? 1 : 2
          assert_equal(expected_allocations, allocated) # dmitry: for some reason, when run locally this numeber is 0, in CI it's 2. IDK why.
        end
      end
    end

    describe "validation" do
      it "accepts built-in method overrides" do
        klass = Class.new do
          extend T::Sig
          sig {params(m: Symbol, include_private: T::Boolean).returns(T::Boolean)}
          def respond_to_missing?(m, include_private=false)
            true
          end
        end

        klass.new.respond_to?(:foo)
      end

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

        @mod.foo {|i| i**2}
      end

      it "allows procs to bind" do
        @mod.sig {params(blk: T.proc.bind(Integer).returns(Integer)).returns(Integer)}
        def @mod.foo(&blk)
          3.instance_eval(&blk)
        end

        assert_equal(7, @mod.foo {self + 4})
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

      class TestEnumerable
        include Enumerable

        def each
          yield "something"
        end
      end

      it "raises a sensible error for custom enumerable validation errors" do
        @mod.sig {returns(T::Array[String])}
        def @mod.foo
          TestEnumerable.new
        end

        err = assert_raises(TypeError) do
          @mod.foo
        end
        assert_match(
          "Return value: Expected type T::Array[String], got Opus::Types::Test::MethodValidationTest::TestEnumerable",
          err.message.lines[0])
      end

      describe 'ranges' do
        describe 'return type is non-nilable integer' do
          it 'permits a range that has integers on start and end' do
            @mod.sig {returns(T::Range[Integer])}
            def @mod.foo
              (1...10)
            end

            assert_equal((1...10), @mod.foo)
          end

          it 'permits a range that has an integer start and no end' do
            @mod.sig {returns(T::Range[Integer])}
            def @mod.foo
              (1...nil)
            end

            assert_equal((1...nil), @mod.foo)
          end

          # Ruby 2.6 does not support ranges with boundless starts
          if RUBY_VERSION >= '2.7'
            it 'permits a range that has an integer start and no end' do
              @mod.sig {returns(T::Range[Integer])}
              def @mod.foo
                (nil...10)
              end

              assert_equal((nil...10), @mod.foo)
            end
          end

          it 'permits a range with no beginning or end' do
            @mod.sig {returns(T::Range[Integer])}
            def @mod.foo
              (nil...nil)
            end

            assert_equal((nil...nil), @mod.foo)
          end
        end
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
              kwopt_bool: T::Boolean,
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
          assert_match(/\AParameter 'kwopt_bool': Expected type T::Boolean, got type Integer/, err.message)
        end

        it "correctly validates a hash passed as the optional arg using implied-hash syntax (treating it as kwargs)" do
          err = assert_raises(TypeError) do
            @mod.foo("foo", kwopt_bool: 42)
          end
          assert_match(/\AParameter 'kwopt_bool': Expected type T::Boolean, got type Integer/, err.message)
        end
      end

      it 'raises a soft_assertion when .on_failure is used with a notify' do
        begin
          T::Configuration.call_validation_error_handler = lambda do |signature, opts|
            if signature.on_failure
              T::Configuration.soft_assert_handler(
                "TypeError: #{opts[:pretty_message]}",
                {notify: signature.on_failure[1][:notify]}
              )
            else
              raise 'test failed'
            end
          end

          @mod.sig {returns(Symbol).on_failure(:soft, notify: 'hello')}
          def @mod.foo
            1
          end

          T::Configuration.expects(:soft_assert_handler).with(
            regexp_matches(/TypeError: Return value: Expected type Symbol, got type Integer with value 1\nCaller: .+\d\nDefinition: .+\d/),
            notify: 'hello'
          )
          @mod.foo
        ensure
          T::Configuration.call_validation_error_handler = nil
        end
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

      it 'handles splats with optional params' do
        @mod.sig {params(bar: String, args: Integer).void}
        def @mod.foo(bar='', *args); end

        @mod.foo('hello')
        @mod.foo('hello', 1, 2, 3)
        @mod.foo

        err = assert_raises(TypeError) do
          @mod.foo(1)
        end
        assert_match(/\AParameter 'bar': Expected type String, got type Integer with value 1/, err.message)
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

      it 'raises an error when two parameters have the same name' do

        @mod.sig {params(_: Integer, _: Integer).returns(String)} # rubocop:disable Lint/DuplicateHashKey
        def @mod.bar(_, _)
          ""
        end

        err = assert_raises(RuntimeError) do
          @mod.bar(0, 0)
        end

        lines = err.message.split("\n")
        assert_equal("The declaration for `bar` has arguments with duplicate names", lines[0])
      end
    end

    describe 'secretly-defined methods with sigs' do
      # The behavior of methods defined via this interface is special: we expect
      # that the methods themselves will perform argument validation.  The sig
      # itself should only be registered to the method so that the rest of sorbet-runtime
      # continues to work "normally".
      it 'should not raise errors on return type mismatch' do
        c = Class.new do
          extend T::Sig

          built_sig = T::Private::Methods._declare_sig(self) do
            returns(Integer)
          end

          T::Private::Methods._with_declared_signature(self, built_sig) do
            def bad_return
              "ok"
            end
          end
        end
        assert_equal("ok", c.new.bad_return)
        # Force the sig block to be actually run.
        T::Utils.signature_for_method(c.instance_method(:bad_return))
        assert_equal("ok", c.new.bad_return)
      end

      it 'should not raise errors on argument type mismatch' do
        c = Class.new do
          extend T::Sig

          built_sig = T::Private::Methods._declare_sig(self) do
            params(x: Integer).returns(Symbol)
          end

          T::Private::Methods._with_declared_signature(self, built_sig) do
            def bad_arg(x)
              :ok
            end
          end
        end
        assert_equal(:ok, c.new.bad_arg("wrong arg type"))
        # Force the sig block to be actually run.
        T::Utils.signature_for_method(c.instance_method(:bad_arg))
        assert_equal(:ok, c.new.bad_arg("wrong arg type"))
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
                kwopt_bool: T::Boolean,
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

        if RUBY_VERSION >= '2.7'
          it "fails if a required kwarg is missing" do
            err = assert_raises(ArgumentError) do
              @mod.foo({})
            end
            assert_equal("missing keyword: :kwreq_int", err.message)
          end
        else
          it "fails if a required kwarg is missing" do
            err = assert_raises(ArgumentError) do
              @mod.foo({})
            end
            assert_equal("missing keyword: kwreq_int", err.message)
          end
        end
      end
    end

    class A
      def foo; end
    end
  end
end
