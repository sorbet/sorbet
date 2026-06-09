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

    it 'falls back for shapes outside the compiled paths' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(Integer) }
        def opt(x, y = 2)
          x + y
        end
        sig { params(x: Integer, rest: Integer).returns(Integer) }
        def splat(x, *rest)
          x
        end
        sig { params(x: Integer, opts: Integer).returns(Integer) }
        def keyrest(x, **opts)
          x
        end
      end
      obj = klass.new
      assert_equal(3, obj.opt(1))
      assert_equal(1, obj.splat(1, 2, 3))
      assert_equal(1, obj.keyrest(1, a: 2))
      refute(compiled?(klass, :opt))
      refute(compiled?(klass, :splat))
      refute(compiled?(klass, :keyrest))
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

  describe 'kwargs compiled dispatch' do
    it 'compiles required-kwargs sigs and validates' do
      klass = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer, y: String).returns(String) }
        def m(s, x:, y:)
          "#{s}-#{x}-#{y}"
        end
      end
      obj = klass.new
      assert_equal("a-1-b", obj.m("a", x: 1, y: "b"))
      assert(compiled?(klass, :m), "expected a compiled kwargs wrapper")

      err = assert_raises(TypeError) { obj.m("a", x: "no", y: "b") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)
      err = assert_raises(TypeError) { obj.m("a", x: 1, y: 2) }
      assert_match(/Parameter 'y': Expected type String, got type Integer/, err.message)
      err = assert_raises(TypeError) { obj.m(9, x: 1, y: "b") }
      assert_match(/Parameter 's': Expected type String, got type Integer/, err.message)
    end

    it 'compiles kwargs-only sigs (no positional args)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x:)
          x * 2
        end
      end
      obj = klass.new
      assert_equal(4, obj.m(x: 2))
      assert(compiled?(klass, :m))
      assert_raises(TypeError) { obj.m(x: "s") }
    end

    it 'applies optional-kwarg defaults in the original (sentinel never leaks, defaults never validated)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, log: T.untyped, y: T.untyped).returns(T.untyped) }
        def m(x:, log: [], y: (log << :default_ran; x + 1))
          [x, y]
        end
      end
      # The `y:` default runs lazily, in the original's own frame (it can
      # even reference the other params), and ONLY when `y:` was omitted.
      obj = klass.new
      side_effects = []
      assert_equal([1, 9], obj.m(x: 1, log: side_effects, y: 9))
      assert_empty(side_effects)
      assert_equal([1, 2], obj.m(x: 1, log: side_effects))
      assert_equal([:default_ran], side_effects)
      assert(compiled?(klass, :m))

      # A default that violates the sig's type is never validated (family
      # parity: only *provided* kwargs are checked).
      klass2 = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.untyped) }
        def m(x: "not an Integer")
          x
        end
      end
      obj2 = klass2.new
      assert_equal("not an Integer", obj2.m)
      assert(compiled?(klass2, :m))
      assert_raises(TypeError) { obj2.m(x: "provided, so validated") }
      assert_equal(3, obj2.m(x: 3))
    end

    it 'dispatches every provision combination of <= 4 optional kwargs without allocating' do
      klass = Class.new do
        extend T::Sig
        sig { params(a: Integer, b: Integer, c: Integer, d: Integer).returns(T::Array[Integer]) }
        def m(a: 1, b: 2, c: 3, d: 4)
          [a, b, c, d]
        end
      end
      obj = klass.new
      names = %i[a b c d]
      defaults = [1, 2, 3, 4]
      (0...16).each do |mask|
        kwargs = {}
        expected = defaults.dup
        names.each_with_index do |name, i|
          if mask[i] == 1
            kwargs[name] = 10 + i
            expected[i] = 10 + i
          end
        end
        assert_equal(expected, obj.m(**kwargs), "provision mask #{mask.to_s(2)}")
      end
      assert(compiled?(klass, :m))
    end

    it 'dispatches >= 5 optional kwargs through all three arms (all/none/partial provision)' do
      klass = Class.new do
        extend T::Sig
        sig { params(a: Integer, b: Integer, c: Integer, d: Integer, e: Integer).returns(T::Array[Integer]) }
        def m(a: 1, b: 2, c: 3, d: 4, e: 5)
          [a, b, c, d, e]
        end
      end
      obj = klass.new
      assert_equal([9, 8, 7, 6, 5], obj.m(a: 9, b: 8, c: 7, d: 6, e: 5)) # all-provided arm
      assert(compiled?(klass, :m))
      assert_equal([1, 2, 3, 4, 5], obj.m)                               # none-provided arm
      assert_equal([1, 7, 3, 4, 50], obj.m(b: 7, e: 50))                 # generic (hash) arm
      assert_equal([0, 2, 3, 4, 5], obj.m(a: 0))
      assert_raises(TypeError) { obj.m(c: "bad") }                       # generic arm still validates
      assert_raises(TypeError) { obj.m(a: 1, b: 2, c: 3, d: 4, e: "bad") } # all-provided arm validates
    end

    it 'mixes required and optional kwargs with required positional args' do
      klass = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer, y: T.nilable(String), z: Integer).returns(String) }
        def m(s, x:, y: nil, z: 9)
          "#{s}|#{x}|#{y.inspect}|#{z}"
        end
      end
      obj = klass.new
      assert_equal('a|1|nil|9', obj.m("a", x: 1))
      assert_equal('a|1|"y"|9', obj.m("a", x: 1, y: "y"))
      assert_equal('a|1|nil|0', obj.m("a", x: 1, z: 0))
      assert_equal('a|1|"y"|0', obj.m("a", x: 1, y: "y", z: 0))
      assert(compiled?(klass, :m))
      assert_raises(TypeError) { obj.m("a", x: 1, y: 5) }
      # explicit nil for a provided T.nilable kwarg is valid
      assert_equal('a|1|nil|9', obj.m("a", x: 1, y: nil))
    end

    it 'raises ArgumentError for missing/unknown keywords and wrong arity like a plain def' do
      klass = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer, y: Integer).returns(Integer) }
        def m(s, x:, y: 2)
          x + y
        end
      end
      obj = klass.new
      obj.m("a", x: 1)
      assert(compiled?(klass, :m))

      err = assert_raises(ArgumentError) { T.unsafe(obj).m("a") }
      assert_equal("missing keyword: :x", err.message)
      err = assert_raises(ArgumentError) { T.unsafe(obj).m("a", x: 1, w: 0) }
      assert_equal("unknown keyword: :w", err.message)
      # Pinned delta (design deltas 3a-3c): the compiled wrapper has the
      # original's real parameter shape, so arity errors carry vanilla
      # Ruby's "; required keyword: x" suffix, exactly like calling the
      # unwrapped def (the family wrapper's `**kwargs` shape drops it).
      err = assert_raises(ArgumentError) { T.unsafe(obj).m("a", "b", x: 1) }
      assert_match(/wrong number of arguments \(given 2, expected 1; required keyword: x\)/, err.message)
      err = assert_raises(ArgumentError) { T.unsafe(obj).m(x: 1) }
      assert_match(/wrong number of arguments \(given 0, expected 1; required keyword: x\)/, err.message)
    end

    it 'reports type objects to the error handler for every parameter (kwargs family parity)' do
      reported = []
      T::Configuration.call_validation_error_handler = lambda do |_sig, opts|
        reported << [opts[:kind], opts[:name], opts[:type]]
      end

      klass = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer, y: T.nilable(String)).returns(T.untyped) }
        def m(s, x:, y: nil)
          :ok
        end
      end
      obj = klass.new
      obj.m("a", x: 1) # compile (first call validates via the interpreter)
      assert(compiled?(klass, :m))
      reported.clear

      obj.m(7, x: 1) # positional: type OBJECT, not the raw Module
      report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :s }
      assert_instance_of(T::Types::Simple, report.fetch(2))
      assert_equal(String, report.fetch(2).raw_type)
      reported.clear

      obj.m("a", x: "bad") # required kwarg: type object
      report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :x }
      assert_instance_of(T::Types::Simple, report.fetch(2))
      assert_equal(Integer, report.fetch(2).raw_type)
      reported.clear

      obj.m("a", x: 1, y: 42) # optional pair-union kwarg: the union type object
      report = reported.find { |(kind, name, _)| kind == 'Parameter' && name == :y }
      assert_instance_of(T::Private::Types::SimplePairUnion, report.fetch(2))
    ensure
      T::Configuration.call_validation_error_handler = nil
    end

    it 'continues after a soft validation error and forwards the provided value (family parity)' do
      T::Configuration.call_validation_error_handler = lambda do |_sig, _opts|
        # swallow
      end
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.untyped) }
        def m(x: 0)
          x
        end
      end
      obj = klass.new
      assert_equal(0, obj.m)
      assert(compiled?(klass, :m))
      assert_equal("soft", obj.m(x: "soft"))
    ensure
      T::Configuration.call_validation_error_handler = nil
    end

    it 'falls back to the families for reserved-word kwarg names, on every call' do
      klass = Class.new do
        extend T::Sig
        sig { params(if: Integer).returns(Integer) }
        def kw_if(if: 1)
          binding.local_variable_get(:if)
        end
        sig { params(end: Integer).returns(Integer) }
        def kw_end(end:)
          binding.local_variable_get(:end)
        end
        sig { params(self: Integer).returns(Integer) }
        def kw_self(self: 2)
          binding.local_variable_get(:self)
        end
        sig { params(__FILE__: Integer).returns(Integer) }
        def kw_file(__FILE__: 3)
          binding.local_variable_get(:__FILE__)
        end
      end
      obj = klass.new
      10.times do
        assert_equal(1, obj.kw_if)
        assert_equal(5, T.unsafe(obj).kw_if(if: 5))
        assert_equal(6, T.unsafe(obj).kw_end(end: 6))
        assert_equal(2, obj.kw_self)
        assert_equal(7, T.unsafe(obj).kw_self(self: 7))
        assert_equal(3, obj.kw_file)
        assert_equal(8, T.unsafe(obj).kw_file(__FILE__: 8))
      end
      refute(compiled?(klass, :kw_if))
      refute(compiled?(klass, :kw_end))
      refute(compiled?(klass, :kw_self))
      refute(compiled?(klass, :kw_file))
      # and the family still validates them
      assert_raises(TypeError) { T.unsafe(obj).kw_if(if: "s") }
      assert_raises(TypeError) { T.unsafe(obj).kw_end(end: "s") }
    end

    it 'falls back for kwarg names that collide with wrapper template locals' do
      klass = Class.new do
        extend T::Sig
        sig { params(return_value: Integer, hash: Integer, arg0: Integer, blk: Integer).returns(T::Array[Integer]) }
        def m(return_value: 1, hash: 2, arg0: 3, blk: 4)
          [return_value, hash, arg0, blk]
        end
      end
      obj = klass.new
      10.times do
        assert_equal([1, 2, 3, 4], obj.m)
        assert_equal([9, 2, 3, 4], obj.m(return_value: 9))
        assert_equal([1, 9, 8, 7], obj.m(hash: 9, arg0: 8, blk: 7))
      end
      refute(compiled?(klass, :m))
      assert_raises(TypeError) { obj.m(hash: "s") }
    end

    it 'falls back for non-ASCII kwarg names' do
      klass = Class.new
      klass.class_eval(<<~RUBY)
        extend T::Sig
        sig { params(é: Integer).returns(Integer) }
        def m(é: 5)
          é
        end
      RUBY
      obj = klass.new
      assert_equal(5, obj.m)
      assert_equal(7, T.unsafe(obj).m(é: 7))
      refute(compiled?(klass, :m))
      assert_raises(TypeError) { T.unsafe(obj).m(é: "s") }
    end

    it 'forwards blocks through kwargs wrappers (literal, pass, identity, break, block_given?)' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.untyped) }
        def m(x:)
          block_given? ? yield(x) : :none
        end
        sig { params(x: Integer, blk: T.nilable(T.proc.returns(Integer))).returns(T.untyped) }
        def cap(x: 0, &blk)
          blk
        end
        sig { params(x: Integer).returns(T.untyped) }
        def brk(x:)
          yield x
          :not_reached
        end
      end
      obj = klass.new
      assert_equal(:none, obj.m(x: 1))
      assert(compiled?(klass, :m))
      assert_equal(10, obj.m(x: 5) { |v| v * 2 })
      pr = proc { 7 }
      obj.cap(x: 1, &pr)
      assert(compiled?(klass, :cap))
      assert_same(pr, obj.cap(x: 1, &pr))
      assert_nil(obj.cap)
      assert_equal(15, obj.brk(x: 5) { |v| break v * 3 })
    end

    it 'supports zsuper through compiled kwargs wrappers (defaults recomputed per level)' do
      parent = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(Integer) }
        def m(x:, y: 10)
          block_given? ? yield(x + y) : x + y
        end
      end
      child = Class.new(parent) do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(Integer) }
        def m(x:, y: 10)
          super
        end
      end
      obj = child.new
      assert_equal(11, obj.m(x: 1))
      assert_equal(3, obj.m(x: 1, y: 2))
      assert_equal(22, obj.m(x: 1) { |v| v * 2 })
      assert(compiled?(parent, :m))
      assert(compiled?(child, :m))
    end

    it 'works through ruby2_keywords delegation' do
      target = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer).returns(String) }
        def t(s, x:)
          "#{s}#{x}"
        end
      end
      delegator = Class.new do
        def initialize(tgt)
          @tgt = tgt
        end
        ruby2_keywords def fwd(*args)
          @tgt.t(*args)
        end
      end
      d = delegator.new(target.new)
      assert_equal("a1", d.fwd("a", x: 1))
      assert(compiled?(target, :t))
      assert_equal("b2", d.fwd("b", x: 2))
      assert_raises(TypeError) { d.fwd("a", x: "bad") }
    end

    it 'keeps visibility for private kwargs methods' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        private def priv(x:)
          x
        end
        def call_priv(v)
          priv(x: v)
        end
      end
      obj = klass.new
      assert_equal(3, obj.call_priv(3))
      assert(compiled?(klass, :priv))
      assert(klass.private_method_defined?(:priv))
      assert_raises(NoMethodError) { T.unsafe(obj).priv(x: 1) }
    end

    it 'compiles kwargs methods behind an eager (boot-time) shim, validating the in-flight first call' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: String).returns(String) }
        def m(x:, y: "d")
          "#{x}#{y}"
        end
      end
      eager_wrap(klass, :m)
      refute(compiled?(klass, :m), "expected a deferred shim, not an immediate compile")

      obj = klass.new
      # The first call (through the shim) must already validate.
      err = assert_raises(TypeError) { obj.m(x: "bad") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)
      assert(compiled?(klass, :m), "the shim's first fire must compile")
      assert_equal("1d", obj.m(x: 1))
      assert_equal("1z", obj.m(x: 1, y: "z"))
      assert_raises(TypeError) { obj.m(x: 1, y: 9) }
    end

    it 'compiles pending kwargs shims via compile_pending!' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x: 3)
          x
        end
      end
      eager_wrap(klass, :m)
      refute(compiled?(klass, :m))
      T::Utils.compile_pending_sig_wrappers!
      assert(compiled?(klass, :m))
      obj = klass.new
      assert_equal(3, obj.m)
      assert_equal(9, obj.m(x: 9))
      assert_raises(TypeError) { obj.m(x: "s") }
    end

    it 'validates complex kwarg types via valid? (and reports the type object)' do
      klass = Class.new do
        extend T::Sig
        sig { params(xs: T::Array[Integer], u: T.untyped).returns(Integer) }
        def m(xs:, u: nil)
          xs.length
        end
      end
      obj = klass.new
      assert_equal(2, obj.m(xs: [1, 2]))
      assert(compiled?(klass, :m))
      assert_equal(0, obj.m(xs: [], u: Object.new))
      err = assert_raises(TypeError) { obj.m(xs: "nope") }
      assert_match(/Parameter 'xs': Expected type T::Array\[Integer\], got type String/, err.message)
    end

    it 'forwards blocks to kwargs bodies reflecting block_given? through aliases (even created after compile)' do
      klass = Class.new do
        extend T::Sig
        alias_method :bg_alias, :block_given?
        sig { params(x: Integer).returns(T::Boolean) }
        def m(x:)
          bg_alias
        end
        sig { params(x: Integer).returns(T::Boolean) }
        def late(x: 0)
          late_alias
        end
      end
      obj = klass.new
      obj.m(x: 1) # compile
      assert(compiled?(klass, :m))
      assert_equal(true, obj.m(x: 1) { :b })
      assert_equal(false, obj.m(x: 1))

      assert_raises(NameError) { obj.late }
      assert(compiled?(klass, :late))
      klass.send(:alias_method, :late_alias, :block_given?)
      assert_equal(true, obj.late { :b })
      assert_equal(false, obj.late)
    end

    it 'falls back (and never renames) for anonymous-module kwarg types' do
      anon = Class.new
      klass = Class.new do
        extend T::Sig
        sig { params(x: anon).returns(T.untyped) }
        def m(x:)
          x
        end
        sig { params(x: T.nilable(anon)).returns(T.untyped) }
        def n(x: nil)
          x
        end
      end
      obj = klass.new
      v = anon.new
      5.times do
        assert_same(v, obj.m(x: v))
        assert_nil(obj.n)
        assert_same(v, obj.n(x: v))
      end
      refute(compiled?(klass, :m))
      refute(compiled?(klass, :n))
      assert_nil(anon.name, "binding must never permanently name the user's module")
      err = assert_raises(TypeError) { obj.m(x: "bad") }
      # the anonymous type's empty display name is preserved exactly
      assert(err.message.start_with?("Parameter 'x': Expected type , got type String"))
      assert_nil(anon.name)
    end

    it 'emits a working kwargs wrapper on the Ruby 3.0 codegen arm (named block forward)' do
      compiled_mod = T::Private::Methods::CallValidation::Compiled
      original = compiled_mod.const_get(:ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.send(:remove_const, :ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.const_set(:ANONYMOUS_BLOCK_FORWARDING, false)
      compiled_mod.send(:private_constant, :ANONYMOUS_BLOCK_FORWARDING)
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer, y: Integer).returns(T.untyped) }
        def m(x:, y: 7)
          block_given? ? yield(x + y) : x + y
        end
      end
      obj = klass.new
      assert_equal(8, obj.m(x: 1)) # compile under the forced 3.0 arm
      assert(compiled?(klass, :m))
      assert_equal(3, obj.m(x: 1, y: 2))
      assert_equal(16, obj.m(x: 1) { |v| v * 2 })
      pr = proc { |v| v - 1 }
      assert_equal(7, obj.m(x: 1, &pr))
      assert_raises(TypeError) { obj.m(x: "s") }
      assert_raises(TypeError) { obj.m(x: 1, y: "s") }
    ensure
      compiled_mod.send(:remove_const, :ANONYMOUS_BLOCK_FORWARDING)
      compiled_mod.const_set(:ANONYMOUS_BLOCK_FORWARDING, original)
      compiled_mod.send(:private_constant, :ANONYMOUS_BLOCK_FORWARDING)
    end

    it 'attributes kwarg type errors to the caller, not to sorbet-runtime frames' do
      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(Integer) }
        def m(x:)
          x
        end
      end
      obj = klass.new
      obj.m(x: 1)
      assert(compiled?(klass, :m))
      err = assert_raises(TypeError) { obj.m(x: "bad") }
      caller_line = err.message.lines.find { |l| l.start_with?("Caller: ") }
      refute_nil(caller_line)
      assert_match(/compiled_dispatch\.rb/, caller_line)
      refute_match(/lib\/types/, caller_line)
    end

    it 'never dispatches equal? on user values in the sentinel test (spoofed/raising equal?, family parity)' do
      # The not-provided test must be `NP.equal?(value)` (the frozen
      # sentinel as receiver, like the family's
      # `ARG_NOT_PROVIDED.equal?(arg0)`), never `value.equal?(NP)`: these
      # two classes spoof/boobytrap `equal?` to prove no user `equal?` ever
      # runs on the dispatch path.
      liar = Class.new do
        def equal?(_other)
          true # claims identity with everything, including the NP sentinel
        end
      end
      raising = Class.new do
        def equal?(_other)
          raise "user equal? ran on the dispatch path"
        end
      end

      klass = Class.new do
        extend T::Sig
        sig { params(x: Integer).returns(T.untyped) }
        def m(x: 42)
          x
        end
        sig { params(u: T.untyped).returns(T.untyped) }
        def fwd(u: :default)
          u
        end
        sig { params(a: Integer, b: Integer, c: Integer, d: Integer, e: Integer).returns(T::Array[T.untyped]) }
        def big(a: 1, b: 2, c: 3, d: 4, e: 5)
          [a, b, c, d, e]
        end
        sig { params(a: T.untyped, b: T.untyped, c: T.untyped, d: T.untyped, e: T.untyped).returns(T::Array[T.untyped]) }
        def big_u(a: 1, b: 2, c: 3, d: 4, e: 5)
          [a, b, c, d, e]
        end
      end
      obj = klass.new
      obj.m
      obj.fwd
      obj.big
      obj.big_u
      assert(compiled?(klass, :m))
      assert(compiled?(klass, :fwd))
      assert(compiled?(klass, :big))
      assert(compiled?(klass, :big_u))

      # An equal?-spoofing value of the wrong type is still type-checked (a
      # value-receiver sentinel test would treat it as not-provided: skip
      # the check and silently return the default 42).
      err = assert_raises(TypeError) { obj.m(x: liar.new) }
      assert_match(/Parameter 'x': Expected type Integer/, err.message)

      # A value whose equal? raises never has it invoked by the wrapper:
      # sorbet's TypeError, not the user's RuntimeError.
      err = assert_raises(TypeError) { obj.m(x: raising.new) }
      assert_match(/Parameter 'x': Expected type Integer/, err.message)

      # A type-valid value with hostile equal? is forwarded -- never
      # dropped for the default, never consulted -- through the <= 4
      # branch-tree dispatch. (object_id compare: assert_same itself calls
      # `expected.equal?`, which these objects sabotage.)
      spoofer = liar.new
      assert_equal(spoofer.object_id, obj.fwd(u: spoofer).object_id)
      bomb = raising.new
      assert_equal(bomb.object_id, obj.fwd(u: bomb).object_id)

      # Same guarantees through every >= 5-optional dispatch arm. Type
      # check first (reviewer repro: base raises TypeError, a spoofable
      # tree returned [1, 2, 3, 4, 50]):
      err = assert_raises(TypeError) { obj.big(b: liar.new, e: 50) }
      assert_match(/Parameter 'b': Expected type Integer/, err.message)
      err = assert_raises(TypeError) { obj.big(b: raising.new, e: 50) }
      assert_match(/Parameter 'b': Expected type Integer/, err.message)
      err = assert_raises(TypeError) { obj.big(a: 9, b: 8, c: 7, d: 6, e: raising.new) }
      assert_match(/Parameter 'e': Expected type Integer/, err.message)
      # Forwarding through the generic (partial-provision, hash-building)
      # arm and the all-provided arm:
      assert_equal([1, spoofer, 3, 4, 50], obj.big_u(b: spoofer, e: 50))
      assert_equal([1, bomb, 3, 4, 50], obj.big_u(b: bomb, e: 50))
      assert_equal([9, 8, 7, 6, bomb], obj.big_u(a: 9, b: 8, c: 7, d: 6, e: bomb))
    end

    it 'pins the delta: multiple invalid kwargs raise/report in declaration order, not provision order' do
      klass = Class.new do
        extend T::Sig
        sig { params(s: String, x: Integer, y: String).returns(T.untyped) }
        def m(s, x:, y: "d")
          [s, x, y]
        end
      end
      obj = klass.new
      obj.m("a", x: 1)
      assert(compiled?(klass, :m))

      # Both provided kwargs are invalid; the caller provides y first. The
      # kwargs family iterates the caller's **kwargs Hash (provision
      # order: it would raise about y); the compiled wrapper checks in
      # declaration order and raises about x. Documented delta (see
      # `wrapper_source`): the reported SET under a soft handler is
      # identical (asserted below), only the order differs -- a single
      # invalid kwarg has no observable difference.
      err = assert_raises(TypeError) { obj.m("a", y: 1, x: "b") }
      assert_match(/Parameter 'x': Expected type Integer, got type String/, err.message)

      reported = []
      T::Configuration.call_validation_error_handler = lambda do |_sig, opts|
        reported << opts[:name]
      end
      assert_equal([7, "b", 1], obj.m(7, y: 1, x: "b"))
      assert_equal(%i[s x y], reported)
    ensure
      T::Configuration.call_validation_error_handler = nil
    end

    it 'falls back for kwarg names with the reserved __t_/__sorbet prefixes' do
      # One method per prefix: a single unsafe name forces the whole method
      # to the family, so bundling them would let either check mask the
      # other.
      klass = Class.new do
        extend T::Sig
        sig { params(__sorbet_x: Integer).returns(Integer) }
        def sorbet_prefixed(__sorbet_x: 1)
          __sorbet_x
        end
        sig { params(__t_y: Integer).returns(Integer) }
        def t_prefixed(__t_y: 2)
          __t_y
        end
      end
      obj = klass.new
      10.times do
        assert_equal(1, obj.sorbet_prefixed)
        assert_equal(5, obj.sorbet_prefixed(__sorbet_x: 5))
        assert_equal(2, obj.t_prefixed)
        assert_equal(6, obj.t_prefixed(__t_y: 6))
      end
      refute(compiled?(klass, :sorbet_prefixed))
      refute(compiled?(klass, :t_prefixed))
      # and the family still validates them
      assert_raises(TypeError) { obj.sorbet_prefixed(__sorbet_x: "s") }
      assert_raises(TypeError) { obj.t_prefixed(__t_y: "s") }
    end
  end
end
