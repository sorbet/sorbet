# frozen_string_literal: true
require_relative '../test_helper'

# Tests for source-compiled sig dispatch (CallValidation::Compiled): the
# plain-`def` wrappers that replace the bind_call validator families for
# fixed-arity positional methods, and in particular the re-entry /
# stale-method-handle state machine ("case file [A]").
class Opus::Types::Test::CompiledDispatchTest < Critic::Unit::UnitTest
  Compiled = T::Private::Methods::CallValidation::Compiled

  private def compiled?(mod, name)
    Compiled.compiled_wrapper?(mod, name)
  end

  # Runs the pending sig block for one specific method in an eager (bulk
  # wrapping) context, exactly like `run_all_sig_blocks` does for every
  # pending sig. (Calling `T::Utils.run_all_sig_blocks` here would drain sig
  # blocks left deliberately broken by other test files in the full suite.)
  private def eager_wrap(mod, name)
    Thread.current[:__t_sig_eager_depth] = (Thread.current[:__t_sig_eager_depth] || 0) + 1
    begin
      T::Private::Methods.run_sig_block_for_method(mod.instance_method(name))
    ensure
      Thread.current[:__t_sig_eager_depth] -= 1
    end
  end

  # The artifact footprint compiled dispatch leaves on a module: one private
  # container constant and one private stash method per compiled generation.
  private def artifact_counts(mod)
    [
      mod.constants(false).count { |c| c.to_s.start_with?("SORBET_RT_SIG_") },
      (mod.private_instance_methods(false) + mod.instance_methods(false))
        .count { |m| m.to_s.start_with?("__t_sig_orig_") },
    ]
  end

  describe 'basic compiled dispatch' do
    it 'compiles eligible fixed-arity positional sigs and validates' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: String).returns(String) }
        def m(x, y)
          "#{x}-#{y}"
        end
      end
      obj = klass.new
      assert_equal("1-a", obj.m(1, "a"))
      assert(compiled?(klass, :m), "expected a compiled wrapper")

      err = assert_raises(TypeError) { obj.m("nope", "a") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)
      err = assert_raises(TypeError) { obj.m(1, 2) }
      assert_match(/Parameter 'y': Expected type String, got type Integer/, err.message)
    end

    it 'validates return values and reports like the families do' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(String) }
        def m(x)
          x # wrong type on purpose
        end
      end
      err = assert_raises(TypeError) { klass.new.m(1) }
      assert_match(/Return value: Expected type String, got type Integer/, err.message)
    end

    it 'handles void, T.anything returns, and zero arity' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).void }
        def v(x); :ignored; end
        sig { params(x: Integer).returns(T.anything) }
        def a(x); x; end
        sig { returns(Integer) }
        def z; 42; end
      end
      obj = klass.new
      assert_equal(T::Private::Types::Void::VOID, obj.v(1))
      assert_equal(5, obj.a(5))
      assert_equal(42, obj.z)
      assert(compiled?(klass, :v))
      assert(compiled?(klass, :a))
      assert(compiled?(klass, :z))
    end

    it 'inlines T.nilable and simple pair unions correctly' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: T.nilable(Integer)).returns(T.nilable(Integer)) }
        def n(x); x; end
        sig { params(x: T.any(Integer, Float)).returns(Integer) }
        def p2(x); x.to_i; end
      end
      obj = klass.new
      assert_nil(obj.n(nil))
      assert_equal(3, obj.n(3))
      err = assert_raises(TypeError) { obj.n("s") }
      assert_match(/Parameter 'x': Expected type T\.nilable\(Integer\), got type String/, err.message)
      assert_equal(1, obj.p2(1.0))
      assert_equal(2, obj.p2(2))
      assert_raises(TypeError) { obj.p2("s") }
      assert(compiled?(klass, :n))
      assert(compiled?(klass, :p2))
    end

    it 'compiles singleton (self.) methods' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def self.m(x); x + 1; end
      end
      assert_equal(2, klass.m(1))
      assert(compiled?(klass.singleton_class, :m))
      assert_raises(TypeError) { klass.m("s") }
    end

    it 'raises plain ArgumentError on arity mismatch like an uncompiled def' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x; end
      end
      obj = klass.new
      obj.m(1)
      err = assert_raises(ArgumentError) { T.unsafe(obj).m(1, 2) }
      assert_match(/wrong number of arguments \(given 2, expected 1\)/, err.message)
    end

    it 'reports the raw type for all-simple sigs and the type object otherwise (family parity)' do
      reported = []
      T::Configuration.call_validation_error_handler = lambda do |_sig, opts|
        reported << [opts[:kind], opts[:name], opts[:type]]
      end

      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def all_simple(x); x; end
        sig { params(x: Integer, y: T.nilable(String)).returns(Integer) }
        def mixed(x, y); x; end
      end
      obj = klass.new
      # Warm up first: the very first call is validated by the generic
      # `validate_call` in the first-call thunk (which always reports type
      # objects), both before and after this feature.
      obj.all_simple(1)
      obj.mixed(1, nil)
      assert(compiled?(klass, :all_simple))
      assert(compiled?(klass, :mixed))
      obj.all_simple("bad")
      param_report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :x }
      assert_equal(Integer, param_report.fetch(2)) # fast family reports the raw Module
      reported.clear

      obj.mixed("bad", nil)
      param_report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :x }
      assert_instance_of(T::Types::Simple, param_report.fetch(2)) # medium family reports the type object
      assert_equal(Integer, param_report.fetch(2).raw_type)
      reported.clear

      obj.mixed(1, 2)
      param_report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :y }
      assert_instance_of(T::Private::Types::SimplePairUnion, param_report.fetch(2))
    ensure
      T::Configuration.call_validation_error_handler = nil
    end

    it 'continues after a soft validation error like the families do' do
      T::Configuration.call_validation_error_handler = lambda do |_sig, _opts|
        # swallow
      end
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); 7; end
      end
      assert_equal(7, klass.new.m("not an int"))
    ensure
      T::Configuration.call_validation_error_handler = nil
    end
  end

  describe 'blocks and super' do
    it 'forwards blocks when the body yields' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x)
          yield x
        end
      end
      assert_equal(10, klass.new.m(5) { |v| v * 2 })
      assert(compiled?(klass, :m))
    end

    it 'preserves block_given? semantics' do
      klass = Class.new do
        extend T::Sig
        sig { returns(T::Boolean) }
        def m
          block_given?
        end
      end
      obj = klass.new
      assert_equal(true, obj.m {})
      assert_equal(false, obj.m)
    end

    it 'ignores blocks passed to methods that never use them (like the original would)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x; end
      end
      assert_equal(1, klass.new.m(1) { raise "never called" })
    end

    it 'validates explicit nilable block params' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, blk: T.nilable(T.proc.returns(Integer))).returns(Integer) }
        def m(x, &blk)
          blk ? blk.call : x
        end
      end
      obj = klass.new
      assert_equal(3, obj.m(3))
      assert_equal(9, obj.m(3) { 9 })
      assert(compiled?(klass, :m))
    end

    it 'supports super (explicit and zsuper) through compiled wrappers, including blocks' do
      parent = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x)
          block_given? ? yield(x) : x + 100
        end
      end
      child = Class.new(parent) do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x)
          super
        end
      end
      obj = child.new
      assert_equal(105, obj.m(5))
      assert_equal(50, obj.m(5) { |v| v * 10 })
      assert(compiled?(parent, :m))
      assert(compiled?(child, :m))
    end

    it 'forwards both block literals and block passes (&proc), preserving identity' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x)
          yield x
        end
        sig { params(x: Integer, blk: T.nilable(T.proc.params(v: Integer).returns(Integer))).returns(T.untyped) }
        def cap(x, &blk)
          blk
        end
      end
      obj = klass.new
      pr = proc { |v| v + 7 }
      assert_equal(8, obj.m(1, &pr))   # block pass
      assert_equal(3, obj.m(1) { |v| v + 2 }) # block literal
      assert(compiled?(klass, :m))
      # A passed Proc must arrive in the body as the SAME object (the
      # bind_call families re-pass the captured Proc; anonymous forwarding
      # must not re-wrap it either).
      obj.cap(1, &pr) # compile
      assert(compiled?(klass, :cap))
      assert_same(pr, obj.cap(1, &pr))
      assert_nil(obj.cap(1))
    end

    it 'supports break through a block forwarded by a compiled wrapper' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x)
          yield x
          x + 1000 # not reached when the block breaks
        end
      end
      obj = klass.new
      obj.m(1) { |v| v } # compile
      assert(compiled?(klass, :m))
      result = obj.m(5) { |v| break v * 3 }
      assert_equal(15, result)
    end

    it 'preserves Proc arity and lambda-ness through forwarded blocks' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, blk: T.nilable(T.proc.params(a: Integer, b: Integer, c: Integer).returns(Integer))).returns(T.untyped) }
        def m(x, &blk)
          blk && [blk.arity, blk.lambda?]
        end
      end
      obj = klass.new
      assert_nil(obj.m(1)) # compile, no block
      assert(compiled?(klass, :m))
      assert_equal([3, false], obj.m(1) { |a, b, c| 0 })
      assert_equal([3, true], obj.m(1, &->(a, b, c) { 0 }))
    end

    it 'forwards blocks to bodies reflecting block_given? through aliases (even created after compile)' do
      klass = Class.new do
        extend T::Sig
        alias_method :bg_alias, :block_given?
        sig { params(x: Integer).returns(T::Boolean) }
        def m(x)
          bg_alias
        end
        sig { params(x: Integer).returns(T::Boolean) }
        def late(x)
          late_alias
        end
      end
      obj = klass.new
      obj.m(1) # compile
      assert(compiled?(klass, :m))
      assert_equal(true, obj.m(1) { :b })
      assert_equal(false, obj.m(1))

      # The alias can be created AFTER the wrapper compiled, so no static
      # analysis of the body could ever justify dropping the block. (The
      # first call's NameError from the missing alias propagates through
      # the wrapper and still compiles it.)
      assert_raises(NameError) { obj.late(1) }
      assert(compiled?(klass, :late))
      klass.send(:alias_method, :late_alias, :block_given?)
      assert_equal(true, obj.late(1) { :b })
      assert_equal(false, obj.late(1))
    end

    it 'forwards blocks observed through an aliased Kernel#method' do
      klass = Class.new do
        extend T::Sig
        alias_method :mth, :method
        sig { params(x: Integer).returns(T::Boolean) }
        def m(x)
          mth(:block_given?).call
        end
      end
      obj = klass.new
      obj.m(1) # compile
      assert(compiled?(klass, :m))
      assert_equal(true, obj.m(1) { :b })
      assert_equal(false, obj.m(1))
    end

    it 'forwards blocks observed through aliased binding/eval' do
      klass = Class.new do
        extend T::Sig
        alias_method :bnd, :binding
        alias_method :evl, :eval
        sig { params(x: Integer).returns(T.untyped) }
        def m(x)
          evl("block_given? ? yield(x) : :none", bnd)
        end
      end
      obj = klass.new
      assert_equal(:none, obj.m(1))
      assert(compiled?(klass, :m))
      assert_equal(8, obj.m(4) { |v| v * 2 })
    end

    it 'emits a working named-block forward on the Ruby 3.0 codegen arm' do
      compiled_mod = T::Private::Methods::CallValidation::Compiled
      # Force the 3.0 emission arm (named `&__t_blk` forward instead of
      # anonymous `&`) and exercise full block semantics through it. This
      # pins that the fallback arm parses and forwards correctly without
      # needing a 3.0 interpreter in CI.
      original = compiled_mod.const_get(:ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.send(:remove_const, :ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.const_set(:ANONYMOUS_BLOCK_FORWARDING, false)
      compiled_mod.send(:private_constant, :ANONYMOUS_BLOCK_FORWARDING)
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.untyped) }
        def m(x)
          block_given? ? yield(x) : :none
        end
      end
      obj = klass.new
      assert_equal(:none, obj.m(1)) # compile under the forced 3.0 arm
      assert(compiled?(klass, :m))
      assert_equal(6, obj.m(3) { |v| v * 2 })
      pr = proc { |v| v - 1 }
      assert_equal(2, obj.m(3, &pr))
      assert_equal(:broke, obj.m(1) { break :broke })
      assert_raises(TypeError) { obj.m("s") }
    ensure
      compiled_mod.send(:remove_const, :ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.const_set(:ANONYMOUS_BLOCK_FORWARDING, original)
      compiled_mod.send(:private_constant, :ANONYMOUS_BLOCK_FORWARDING)
    end
  end

  describe 'visibility' do
    it 'keeps private and protected visibility through compile' do
      klass = Class.new do
        extend T::Sig
        sig { returns(Symbol) }
        private def priv; :p; end
        sig { returns(Symbol) }
        def prot; :q; end
        protected :prot
        def call_both(other)
          [priv, other.prot]
        end
      end
      a = klass.new
      assert_equal(%i[p q], a.call_both(klass.new))
      assert_raises(NoMethodError) { T.unsafe(a).priv }
      assert_raises(NoMethodError) { T.unsafe(a).prot }
      assert(compiled?(klass, :priv))
      assert(compiled?(klass, :prot))
    end

    it 'keeps an explicitly-public initialize public through compile' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).void }
        def initialize(x)
          @x = x
        end
        public :initialize
      end
      klass.new(1)
      assert(compiled?(klass, :initialize))
      assert(klass.public_method_defined?(:initialize))
    end

    it 'keeps a default-private initialize private through compile' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).void }
        def initialize(x)
          @x = x
        end
      end
      klass.new(1)
      assert(compiled?(klass, :initialize))
      assert(klass.private_method_defined?(:initialize))
    end

    it 'preserves module_function dual copies (validation, visibility, blocks)' do
      mod = Module.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        module_function def mf(x)
          block_given? ? yield(x) : x + 1
        end
      end

      # Singleton copy: public, validates, forwards blocks.
      assert_equal(2, mod.mf(1))
      assert_equal(10, mod.mf(1) { |v| v * 10 })
      assert_raises(TypeError) { T.unsafe(mod).mf("s") }

      # Instance copy: private, validates, forwards blocks.
      obj = Class.new { include mod }.new
      assert_equal(6, obj.send(:mf, 5))
      assert_equal(0, obj.send(:mf, 5) { |v| v - 5 })
      assert_raises(TypeError) { obj.send(:mf, "s") }
      assert(mod.private_instance_methods.include?(:mf))
      assert_raises(NoMethodError) { T.unsafe(obj).mf(5) }
    end
  end

  describe 'eligibility fallbacks' do
    it 'falls back to the families for operator and reserved-word method names' do
      klass = Class.new do
        extend T::Sig
        sig { params(other: Integer).returns(T::Boolean) }
        def ==(other)
          other == 1
        end

        sig { params(x: Integer).returns(Integer) }
        def if(x)
          x
        end
      end
      obj = klass.new
      assert_equal(true, obj == 1)
      assert_equal(5, obj.send(:if, 5))
      refute(compiled?(klass, :==))
      refute(compiled?(klass, :if))
      # but they still validate, via the families
      assert_raises(TypeError) { obj == "s" }
      assert_raises(TypeError) { obj.send(:if, "s") }
    end

    it 'falls back for shapes outside the positional fast path' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(Integer) }
        def kw(x, y: 2)
          x + y
        end
        sig { params(x: Integer, rest: Integer).returns(Integer) }
        def splat(x, *rest)
          x
        end
      end
      obj = klass.new
      assert_equal(3, obj.kw(1))
      assert_equal(1, obj.splat(1, 2, 3))
      refute(compiled?(klass, :kw))
      refute(compiled?(klass, :splat))
    end
  end

  describe 'case file [A]: stale handles, idempotency, zero growth' do
    it 'a stale pre-wrap Method handle converges with zero artifact growth' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      obj = klass.new
      stale = obj.method(:m) # handle to the first-call thunk

      assert_equal(2, obj.m(1)) # compiles
      assert(compiled?(klass, :m))
      counts = artifact_counts(klass)
      live_src = klass.instance_method(:m).source_location

      1_000.times do
        assert_equal(3, stale.call(2))
      end

      assert_equal(counts, artifact_counts(klass), "stale handle calls must not grow artifacts")
      assert_equal(live_src, klass.instance_method(:m).source_location, "stale handle calls must not redefine the method")
      assert_equal(2, obj.m(1))
      assert_raises(TypeError) { stale.call("s") } # stale calls still validate
    end

    it 'alternating stale handles after a sigged redefinition do not clobber the new body or grow artifacts' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      obj = klass.new
      h1 = obj.method(:m) # thunk for the first def

      klass.class_eval do
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 100; end
      end
      h2 = obj.method(:m) # thunk for the second def

      assert_equal(101, obj.m(1)) # compiles the redefined method
      assert(compiled?(klass, :m))
      counts = artifact_counts(klass)
      live_src = klass.instance_method(:m).source_location

      500.times do
        # The in-flight stale call dispatches its own (old) original -- same
        # as before this feature -- but must not redefine the live method.
        assert_equal(2, h1.call(1))
        assert_equal(101, h2.call(1))
      end

      assert_equal(counts, artifact_counts(klass), "stale ping-pong must not grow artifacts")
      assert_equal(live_src, klass.instance_method(:m).source_location)
      assert_equal(101, obj.m(1), "normal calls must keep running the NEW body")
    end

    it 'repeated public wrap_method_with_call_validation_if_needed calls with the registered sig are no-ops' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x; end
      end
      obj = klass.new
      obj.m(1)
      assert(compiled?(klass, :m))
      registered_sig = T::Private::Methods.signature_for_method(klass.instance_method(:m))
      refute_nil(registered_sig)

      counts = artifact_counts(klass)
      live_src = klass.instance_method(:m).source_location
      100.times do
        T::Utils.wrap_method_with_call_validation_if_needed(klass, registered_sig, klass.instance_method(:m))
      end
      assert_equal(counts, artifact_counts(klass))
      assert_equal(live_src, klass.instance_method(:m).source_location)
      assert_equal(1, obj.m(1))
      assert_raises(TypeError) { T.unsafe(obj).m("s") }
    end

    it 'public wrap with a DIFFERENT sig still stacks a validator (today-parity)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.anything) }
        def m(x); x; end
      end
      obj = klass.new
      obj.m(1)
      assert(compiled?(klass, :m))

      # The live compiled wrapper declares a block parameter (it always
      # forwards), just like the bind_call family wrappers do; pass explicit
      # `parameters:` so the declaration only needs to cover the positional.
      new_sig = T::Private::Methods::Signature.new(
        method: klass.instance_method(:m),
        method_name: :m,
        raw_arg_types: {arg0: String},
        raw_return_type: T.anything,
        bind: nil,
        mode: T::Private::Methods::Modes.standard,
        check_level: :always,
        on_failure: nil,
        parameters: [[:req, :arg0]],
      )
      T::Utils.wrap_method_with_call_validation_if_needed(klass, new_sig, klass.instance_method(:m))

      # Outer validator enforces the new sig...
      err = assert_raises(TypeError) { obj.m(1) }
      assert_match(/Expected type String, got type Integer/, err.message)
      # ...and the inner compiled wrapper still enforces the original sig.
      err = assert_raises(TypeError) { T.unsafe(obj).m("s") }
      assert_match(/Expected type Integer, got type String/, err.message)
    end

    it 'aliases get their own compiled validator without cross-poisoning' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def a(x); x + 1; end
        alias_method :b, :a
      end
      obj = klass.new
      assert_equal(2, obj.b(1)) # alias slow path, then fast validator for :b
      assert_equal(2, obj.a(1))
      assert_equal(2, obj.b(1))
      assert(compiled?(klass, :a))
      assert(compiled?(klass, :b))
      assert_raises(TypeError) { obj.a("s") }
      assert_raises(TypeError) { obj.b("s") }
    end

    it 'an unsigged redefinition followed by a stale-handle call clobbers (today-parity) but stays bounded' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      obj = klass.new
      stale = obj.method(:m)
      assert_equal(2, obj.m(1))

      T::Configuration.without_ruby_warnings do
        klass.class_eval do
          def m(x); x + 50; end # plain redefinition, no sig
        end
      end
      assert_equal(51, obj.m(1))

      # Today-parity: the stale handle re-installs validated dispatch around
      # the OLD original, clobbering the unsigged redefinition.
      assert_equal(2, stale.call(1))
      assert_equal(2, obj.m(1))

      # But repeated stale calls converge (no per-call redefinition/growth).
      counts = artifact_counts(klass)
      live_src = klass.instance_method(:m).source_location
      200.times { assert_equal(2, stale.call(1)) }
      assert_equal(counts, artifact_counts(klass))
      assert_equal(live_src, klass.instance_method(:m).source_location)
    end

    it 'keeps working under Module#prepend, with zero owner artifact growth from stale calls' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      obj = klass.new
      stale = obj.method(:m)
      assert_equal(2, obj.m(1)) # compile first
      counts = artifact_counts(klass)

      interceptor = Module.new do
        def m(x)
          super(x) * 10
        end
      end
      klass.prepend(interceptor)

      assert_equal(20, obj.m(1)) # prepended override + compiled wrapper via super
      assert_raises(TypeError) { obj.m("s") }

      # Today-parity (verified identical on the base commit): a stale
      # pre-wrap handle call retargets the wrap at the *prepended* module,
      # which is ineligible for compilation (it doesn't own the original),
      # so a family validator replaces the prepended override there. The
      # owner's compiled artifacts must not grow while this happens.
      assert_equal(2, stale.call(1))
      50.times { stale.call(1) }
      assert_equal(counts, artifact_counts(klass), "stale calls under prepend must not grow owner artifacts")
      assert_equal(2, obj.m(1)) # today-parity: family validator now owns the prepended slot
      assert_raises(TypeError) { obj.m("s") } # and it still validates
    end

    it 'is thread-safe on concurrent first calls' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x * 2; end
      end
      obj = klass.new
      results = Array.new(8)
      threads = 8.times.map do |i|
        Thread.new do
          100.times { results[i] = obj.m(21) }
        end
      end
      threads.each(&:join)
      assert_equal([42] * 8, results)
      assert_raises(TypeError) { obj.m("s") }
    end
  end

  describe 'eager tier (run_all_sig_blocks) and eval policy' do
    it 'installs a shim during run_all_sig_blocks and compiles on first call' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 2; end
      end
      eager_wrap(klass, :m)
      refute(compiled?(klass, :m), "eager wrap should install a shim, not compile")
      assert_equal(3, klass.new.m(1))
      assert(compiled?(klass, :m), "first call through the shim should compile")
      assert_raises(TypeError) { klass.new.m("s") }
    end

    it 'validates the in-flight first call through the shim' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x; end
      end
      eager_wrap(klass, :m)
      err = assert_raises(TypeError) { klass.new.m("bad") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)
    end

    it 'compile_pending_sig_wrappers! compiles shimmed methods without calling them' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 3; end
      end
      eager_wrap(klass, :m)
      refute(compiled?(klass, :m))
      T::Utils.compile_pending_sig_wrappers!
      assert(compiled?(klass, :m))
      assert_equal(4, klass.new.m(1))
      assert_raises(TypeError) { klass.new.m("s") }
    end

    it 'compile_pending_sig_wrappers! does not resurrect a shim over a later unsigged redefinition' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      eager_wrap(klass, :m)
      T::Configuration.without_ruby_warnings do
        klass.class_eval do
          def m(x); x + 1000; end # user replaces the shimmed method, unsigged
        end
      end
      T::Utils.compile_pending_sig_wrappers!
      refute(compiled?(klass, :m))
      assert_equal(1001, klass.new.m(1), "the user's replacement must stay live")
    end

    it 'falls back to the families after disable_lazy_evaluation! (no eval), and stays correct' do
      T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 4; end
      end
      assert_equal(5, klass.new.m(1))
      refute(compiled?(klass, :m), "no source may be compiled after disable_lazy_evaluation!")
      assert_raises(TypeError) { klass.new.m("s") }
    ensure
      if T::Props::HasLazilySpecializedMethods.instance_variable_defined?(:@lazy_evaluation_disabled)
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end

    it 'compile_pending_sig_wrappers! still works after disable_lazy_evaluation! (explicit-hook exemption)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 5; end
      end
      eager_wrap(klass, :m)
      T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
      T::Utils.compile_pending_sig_wrappers!
      assert(compiled?(klass, :m))
      assert_equal(6, klass.new.m(1))
    ensure
      if T::Props::HasLazilySpecializedMethods.instance_variable_defined?(:@lazy_evaluation_disabled)
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end
  end

  describe 'eager tier hardening' do
    it 'keeps validating through the shim forever when the owner is frozen after boot wrapping' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      eager_wrap(klass, :m)
      obj = klass.new
      klass.freeze

      # No call may raise FrozenError; the shim validates every call.
      3.times { |i| assert_equal(i + 2, obj.m(i + 1)) }
      refute(compiled?(klass, :m), "a frozen owner can never be compiled")
      err = assert_raises(TypeError) { obj.m("s") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)
      3.times { assert_equal(2, obj.m(1)) }

      # compile_pending! must skip (not raise on) frozen owners, and must
      # not break the still-live shim.
      T::Utils.compile_pending_sig_wrappers!
      assert_equal(2, obj.m(1))
      assert_raises(TypeError) { obj.m("s") }
    end

    it 'does not clobber an alias_method-decoration of a shimmed method (first call fires the old shim)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      eager_wrap(klass, :m)

      T::Configuration.without_ruby_warnings do
        klass.class_eval do
          alias_method(:m_without_trace, :m)
          def m(x)
            m_without_trace(x) + 1000
          end
        end
      end

      counts = artifact_counts(klass)
      # Every call goes new body -> alias'd shim (stale; inert) -> validated
      # old original. The decoration must survive every call, with zero
      # artifact growth from the stale shim fires.
      results = 5.times.map { klass.new.m(1) }
      assert_equal([1002] * 5, results, "alias decoration must not be clobbered")
      assert_equal(counts, artifact_counts(klass))
      assert_raises(TypeError) { klass.new.m("s") }
    end

    it 'a stale handle to the boot shim cannot retarget name dispatch after an unsigged redefinition' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      eager_wrap(klass, :m)
      obj = klass.new
      h = obj.method(:m) # handle to the boot shim

      T::Configuration.without_ruby_warnings do
        klass.class_eval do
          def m(x); x * 100; end # unsigged replacement
        end
      end

      assert_equal(100, obj.m(1))
      # The stale handle still runs the old original (validated, in-flight
      # semantics) -- on base and patched alike...
      assert_equal(2, h.call(1))
      # ...but it must NOT have rewritten what the NAME dispatches to.
      assert_equal(100, obj.m(1), "name dispatch must keep the user's replacement")

      counts = artifact_counts(klass)
      live = klass.instance_method(:m)
      200.times { |i| assert_equal(i + 2, h.call(i + 1)) }
      assert_equal(counts, artifact_counts(klass), "stale shim fires must not grow artifacts")
      assert_equal(live, klass.instance_method(:m), "stale shim fires must not redefine the method")
      assert_equal(100, obj.m(1))
      assert_raises(TypeError) { h.call("s") } # stale calls still validate
    end

    it 'keeps an explicitly-public initialize public during the shim window and after compile' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).void }
        def initialize(x)
          @x = x
        end
        public :initialize
      end
      eager_wrap(klass, :initialize)
      assert(klass.public_method_defined?(:initialize), "shim must preserve explicit public visibility")
      instance = klass.allocate
      instance.initialize(1) # fires the shim; compiles
      assert(klass.public_method_defined?(:initialize), "compile must preserve explicit public visibility")
      assert_raises(TypeError) { klass.allocate.initialize("s") }
    end

    it 'a first call racing a sigged redefinition re-shim surrenders instead of resurrecting the old body' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 1; end
      end
      eager_wrap(klass, :m) # shim1 + pending1

      pending = Compiled.instance_variable_get(:@pending_shims)
      key = "#{klass.object_id}#m"
      ps1 = pending[key]
      refute_nil(ps1)
      orig1, sig1, vis1 = ps1.orig, ps1.sig, ps1.visibility

      # Sigged redefinition, re-wrapped eagerly: shim2 + pending2 own the key.
      T::Configuration.without_ruby_warnings do
        klass.class_eval do
          extend T::Sig
          sig { params(x: Integer).returns(Integer) }
          def m(x); x * 100; end
        end
      end
      eager_wrap(klass, :m)
      ps2 = pending[key]
      refute(ps2.equal?(ps1), "redefinition must have re-shimmed the key")
      shim2 = Compiled.own_method(klass, :m)

      # Replay thread A resuming inside shim_fired after the re-shim
      # (the reported interleaving, step 4): must install nothing.
      Compiled.shim_fired(klass, :m, orig1, sig1, vis1)
      assert_equal(shim2, Compiled.own_method(klass, :m), "shim_fired must not clobber the newer shim")
      assert(pending[key].equal?(ps2), "the newer pending record must survive")

      # Replay thread A resuming inside compile_now! (already past
      # shim_fired's guards): the :shimmed registry arm must surrender.
      assert_equal(:stale, Compiled.compile_now!(klass, :m, orig1, sig1, vis1))
      assert_equal(shim2, Compiled.own_method(klass, :m))
      assert(pending[key].equal?(ps2))

      # Replay thread A already past the state-machine read (claimed before
      # seeing the re-shim): build_wrapper's post-claim pending guard.
      assert_equal(:stale, Compiled.send(:build_wrapper, klass, key, :m, orig1, sig1, vis1, 1))
      assert_equal(shim2, Compiled.own_method(klass, :m))
      assert(pending[key].equal?(ps2))

      # The new method's own first call wins and compiles the NEW body.
      assert_equal(100, klass.new.m(1))
      assert(compiled?(klass, :m))
      assert_equal(200, klass.new.m(2))
      assert_raises(TypeError) { klass.new.m("s") }
    end
  end

  describe 'validation semantics parity' do
    it 'rejects null-objects that override nil? for T.nilable params and returns' do
      null_klass = Class.new do
        def nil?; true; end
      end
      klass = Class.new do
        extend T::Sig
        sig { params(x: T.nilable(Integer)).returns(T.nilable(Integer)) }
        def m(x); 1; end
        sig { params(x: Integer, sneak: T.untyped).returns(T.nilable(Integer)) }
        def r(x, sneak); x == 1 ? nil : sneak; end
      end
      obj = klass.new
      assert_equal(1, obj.m(nil)) # compile; nil accepted
      assert_nil(obj.r(1, nil))   # compile
      assert(compiled?(klass, :m))
      assert(compiled?(klass, :r))
      # `SimplePairUnion#valid?` is `is_a?(NilClass) || is_a?(Integer)`; an
      # overridden `nil?` must not be consulted (base parity).
      assert_raises(TypeError) { obj.m(null_klass.new) }
      assert_raises(TypeError) { obj.r(2, null_klass.new) }
    end

    it 'forwards blocks to bodies that reflect block_given? through Kernel#method' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T::Boolean) }
        def m(x)
          method(:block_given?).call
        end
      end
      obj = klass.new
      obj.m(1) # compile
      assert_equal(true, obj.m(1) { :blk })
      assert_equal(false, obj.m(1))
    end

    it 'attributes prop setter type errors to the user call site, never to gem-internal frames' do
      struct = Class.new(T::Struct) do
        prop :x, Integer
      end
      s = struct.new(x: 1)
      # `SetterFactory.raise_pretty_error` is itself sigged; after the first
      # error its own sig wraps (and, when eligible, source-compiles), so a
      # wrapper frame -- a family bmethod at call_validation_2_7.rb, the
      # validate_call interpreter, or a compiled wrapper at a
      # `call_validation_compiled.rb(compiled:...)` pseudo-path -- sits
      # directly above it. None of those may ever be reported as "Caller":
      # every frame under the gem's lib root is skipped. Iterate a few times
      # so the assertion covers the interpreted first call, the wrap, and
      # the steady state.
      3.times do
        line = nil
        err = assert_raises(TypeError) do
          line = __LINE__ + 1
          s.x = "bad"
        end
        assert_match(/^Caller: #{Regexp.escape(__FILE__)}:#{line}$/, err.message)
      end
    end

    it 'never compiles refinement modules (version-portable detection) but still validates them' do
      refinement = nil
      Module.new do
        # `refine` returns the refinement module on every supported Ruby;
        # `Module#refinements` only exists on >= 3.2.
        refinement = refine(String) do
          extend T::Sig
          sig { params(x: Integer).returns(String) }
          def rep(x); self * x; end
        end
      end
      assert(Compiled.refinement_module?(refinement))
      # The Ruby 3.0/3.1 floor has no ::Refinement class; the to_s marker is
      # the only portable signal there. Pin that it holds.
      assert(Module.instance_method(:to_s).bind_call(refinement).start_with?("#<refinement:"))
      refute(Compiled.refinement_module?(String))
      refute(Compiled.refinement_module?(Module.new))

      # Run the sig block (what the first call through an activated
      # refinement would do): the wrap must take the family-validator path
      # -- a compiled wrapper's receiverless stash call cannot resolve on a
      # refinement module and would NoMethodError on every call.
      T::Private::Methods.run_sig_block_for_method(refinement.instance_method(:rep))
      refute(Compiled.compiled_wrapper?(refinement, :rep))
      assert_equal("abab", refinement.instance_method(:rep).bind_call("ab", 2))
      assert_equal("ababab", refinement.instance_method(:rep).bind_call("ab", 3))
      assert_raises(TypeError) { refinement.instance_method(:rep).bind_call("ab", "s") }
    end
  end

  describe 'anonymous-module sig types (rename guard)' do
    # `const_set` permanently renames any module that does not already have a
    # permanent (constant-path) name, so binding an anonymous raw type into a
    # wrapper's container constant would globally mutate the user's module
    # (`name`/`to_s`/`inspect`) and change validation error text. Such sigs
    # must take the family validators on every call.

    it 'falls back, validates calls 1..N, and never renames anonymous param/return types' do
      anon = Class.new
      klass = Class.new do
        extend T::Sig
        sig { params(x: anon).returns(anon) }
        def m(x); x; end

        sig { params(x: T.untyped).returns(anon) }
        def r(x); x; end
      end
      obj = klass.new
      inst = anon.new
      50.times { assert_same(inst, obj.m(inst)) }
      50.times { assert_same(inst, obj.r(inst)) }
      refute(compiled?(klass, :m), "anonymous-typed sigs must take the family path")
      refute(compiled?(klass, :r))

      # The user module's identity must be untouched (base parity)...
      assert_nil(anon.name)
      assert_nil(Module.instance_method(:name).bind_call(anon))
      refute_match(/SORBET_RT_SIG/, anon.to_s)
      assert_equal([0, 0], artifact_counts(klass), "no compiled artifacts may be left on the owner")
      assert_nil(Compiled.instance_variable_get(:@compiled)["#{klass.object_id}#m"])

      # ...and the error text must be byte-identical to base (`Simple#name`
      # for an anonymous raw module is the empty string).
      err = assert_raises(TypeError) { obj.m("bad") }
      assert_includes(err.message, "Parameter 'x': Expected type , got type String")
      refute_match(/SORBET_RT_SIG/, err.message)
      err = assert_raises(TypeError) { obj.r("bad") }
      assert_includes(err.message, "Return value: Expected type , got type String")
      assert_nil(anon.name, "raising must not have named the module either")
    end

    it 'keeps T.nilable(anonymous) un-renamed with base error text' do
      anon = Class.new
      klass = Class.new do
        extend T::Sig
        sig { params(x: T.nilable(anon)).returns(T.nilable(anon)) }
        def n(x); x; end
      end
      obj = klass.new
      inst = anon.new
      25.times do
        assert_nil(obj.n(nil))
        assert_same(inst, obj.n(inst))
      end
      refute(compiled?(klass, :n))
      assert_nil(anon.name)
      err = assert_raises(TypeError) { obj.n("bad") }
      assert_includes(err.message, "Parameter 'x': Expected type T.nilable(), got type String")
      refute_match(/SORBET_RT_SIG/, err.message)
      assert_nil(anon.name)
    end

    it 'never renames through the eager tier or compile_pending!' do
      anon = Class.new
      klass = Class.new do
        extend T::Sig
        sig { params(x: anon).returns(anon) }
        def m(x); x; end
      end
      eager_wrap(klass, :m)
      refute(compiled?(klass, :m))
      inst = anon.new
      10.times { assert_same(inst, klass.new.m(inst)) }
      T::Utils.compile_pending_sig_wrappers!
      refute(compiled?(klass, :m))
      10.times { assert_same(inst, klass.new.m(inst)) }
      assert_nil(anon.name)
      assert_equal([0, 0], artifact_counts(klass))
      assert_raises(TypeError) { klass.new.m("bad") }
      assert_nil(anon.name)
    end

    it 'refuses at the binding site too (direct compile_now! without the wrap-time gate)' do
      anon = Class.new
      klass = Class.new do
        def m(x); x; end
      end
      sig = T::Private::Methods::Signature.new(
        method: klass.instance_method(:m),
        method_name: :m,
        raw_arg_types: {x: anon},
        raw_return_type: anon,
        bind: nil,
        mode: T::Private::Methods::Modes.standard,
        check_level: :always,
        on_failure: nil,
      )
      before = klass.instance_method(:m)
      assert_nil(Compiled.compile_now!(klass, :m, klass.instance_method(:m), sig, :public))
      assert_equal(before, klass.instance_method(:m), "a refused build must not touch the method table")
      assert_nil(anon.name, "a refused build must not have bound (and renamed) the module")
      assert_equal([0, 0], artifact_counts(klass))
      # Terminal: the registry holds a :failed entry; a retry stays refused.
      assert_nil(Compiled.compile_now!(klass, :m, klass.instance_method(:m), sig, :public))
      assert_nil(anon.name)
    end

    it 'also refuses pseudo-named modules (rebinding those renames them as well)' do
      root = Module.new
      pseudo = Class.new
      root.const_set(:Inner, pseudo) # name is now "#<Module:0x...>::Inner"
      name_before = Module.instance_method(:name).bind_call(pseudo)
      refute_nil(name_before) # not anonymous -- but not permanently named either
      refute(Compiled.send(:permanently_named?, pseudo))

      klass = Class.new do
        extend T::Sig
        sig { params(x: pseudo).returns(pseudo) }
        def m(x); x; end
      end
      obj = klass.new
      inst = pseudo.new
      10.times { assert_same(inst, obj.m(inst)) }
      refute(compiled?(klass, :m))
      assert_equal(name_before, Module.instance_method(:name).bind_call(pseudo))
      refute_match(/SORBET_RT_SIG/, pseudo.name)
      assert_raises(TypeError) { obj.m("bad") }
    end

    it 'is not over-broad: permanently-named types (including nested paths) still compile' do
      assert(Compiled.send(:permanently_named?, Integer))
      assert(Compiled.send(:permanently_named?, T::Private::Types::Void))
      refute(Compiled.send(:permanently_named?, Class.new))
      refute(Compiled.send(:permanently_named?, Module.new))

      klass = Class.new do
        extend T::Sig
        sig { params(x: T::Private::Types::Void, y: T.nilable(Integer)).returns(Integer) }
        def m(x, y); 1; end
      end
      obj = klass.new
      assert_equal(1, obj.m(T::Private::Types::Void.new, nil))
      assert(compiled?(klass, :m), "named nested-path types must still compile")
      assert_raises(TypeError) { obj.m("bad", nil) }
    end
  end

  describe 'kill switch' do
    it 'disable! falls back to the families for new wraps' do
      Compiled.disable!
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x + 6; end
      end
      assert_equal(7, klass.new.m(1))
      refute(compiled?(klass, :m))
      assert_raises(TypeError) { klass.new.m("s") }
    ensure
      Compiled.enable!
    end
  end

  describe 'introspection' do
    it 'signature_for_method still finds the sig after compile' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x); x; end
      end
      klass.new.m(1)
      assert(compiled?(klass, :m))
      sig = T::Utils.signature_for_method(klass.instance_method(:m))
      refute_nil(sig)
      assert_equal(:m, sig.method_name)
    end

    it 'keeps arity and required positional parameters' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(Integer) }
        def m(x, y); x + y; end
      end
      klass.new.m(1, 2)
      assert(compiled?(klass, :m))
      assert_equal(2, klass.instance_method(:m).arity)
      assert_equal(%i[req req], klass.instance_method(:m).parameters.map(&:first).reject { |k| k == :block })
    end
  end
end
