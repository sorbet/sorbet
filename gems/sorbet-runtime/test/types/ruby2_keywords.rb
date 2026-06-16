# frozen_string_literal: true
# typed: ignore
require_relative '../test_helper'

class Opus::Types::Test::Ruby2KeywordsTest < Critic::Unit::UnitTest
  # Verifies that sig-wrapped methods do not emit ruby2_keywords warnings on
  # Ruby 3.4+, where the VM warns if ruby2_keywords is applied to a method that
  # already handles keywords or does not accept a splat.

  def warnings_for
    old_stderr = $stderr
    $stderr = StringIO.new
    yield
    $stderr.string
  ensure
    $stderr = old_stderr
  end

  it "does not warn for a plain method with no splat" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer, b: Integer).returns(Integer) }
      def add(a, b) = a + b
    end

    output = warnings_for { klass.new.add(1, 2) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with only keyword args" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer).returns(Integer) }
      def double(a:) = a * 2
    end

    output = warnings_for { klass.new.double(a: 5) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with splat and keyword args" do
    klass = Class.new do
      extend T::Sig
      sig { params(args: Integer, opts: String).returns(NilClass) }
      def mixed(*args, **opts) = nil
    end

    output = warnings_for { klass.new.mixed(1, 2, key: "val") }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with only a splat" do
    klass = Class.new do
      extend T::Sig
      sig { params(args: Integer).returns(NilClass) }
      def splat_only(*args) = nil
    end

    output = warnings_for { klass.new.splat_only(1, 2, 3) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "correctly calls methods after sig wrapping" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer, b: Integer).returns(Integer) }
      def add(a, b) = a + b

      sig { params(args: Integer).returns(Integer) }
      def sum(*args) = args.sum

      sig { params(a: Integer).returns(Integer) }
      def double(a:) = a * 2
    end

    instance = klass.new
    assert_equal 3, instance.add(1, 2)
    assert_equal 6, instance.sum(1, 2, 3)
    assert_equal 10, instance.double(a: 5)
  end

  # Regression: optimized wrappers must hand arguments to the wrapped method with the same
  # ruby2_keywords flagging the slow `|*args, &blk|` validator would. When an upstream ruby2_keywords
  # splat forwards `key: val` into a sig'd method that binds it to a named positional parameter, Ruby
  # strips the keyword flag as the value lands in the positional slot. A specialized wrapper binding
  # through its own `*rest`/`**kwargs` instead preserved the flag, so a downstream re-forwarder
  # re-splatted the hash as keywords and raised `missing keywords` / `wrong number of arguments`.
  # `Hash.ruby2_keywords_hash?` asserts the flag state directly; the expected state is stripped (false),
  # matching a plain `def` and the slow validator.

  # Builds a ruby2_keywords-flagged forwarder that calls `create` on `target` directly. The single hop
  # matters: an intermediate non-flagged call (e.g. `public_send`) would strip the flag before it reached
  # the sig wrapper, masking the regression.
  def create_forwarder(target)
    mod = Module.new do
      define_method(:fwd) { |*args| target.create(*args) }
      ruby2_keywords(:fwd)
    end
    Object.new.extend(mod)
  end

  it "strips the ruby2_keywords flag when binding to an optional positional (Mutator.create shape)" do
    klass = Class.new do
      extend T::Sig
      sig do
        params(params: T::Hash[T.untyped, T.untyped], opts: T::Hash[T.untyped, T.untyped])
          .returns(T.untyped)
          .checked(:always)
      end
      # Mirrors Mutator.create(params = {}, opts = {}, &blk).
      def create(params={}, opts={})
        [Hash.ruby2_keywords_hash?(params), params]
      end
    end

    instance = klass.new
    fwd = create_forwarder(instance)
    5.times { fwd.fwd(warm: 1) } # install the optimized wrapper

    # The trailing keyword binds to the positional `params`, so the flag must be stripped (as a plain
    # `def` would) -- a downstream re-forwarder must see a positional hash, not keywords.
    flagged, params = fwd.fwd(task: "do_thing", event: "evt_123")
    refute(flagged, "expected the ruby2_keywords flag to be stripped")
    assert_equal({task: "do_thing", event: "evt_123"}, params)
  end

  it "preserves positional hash arguments for an optional-positional method" do
    klass = Class.new do
      extend T::Sig
      sig do
        params(params: T::Hash[T.untyped, T.untyped], opts: T::Hash[T.untyped, T.untyped])
          .returns(T.untyped)
          .checked(:always)
      end
      def create(params={}, opts={})
        [params, opts]
      end
    end

    instance = klass.new
    3.times { instance.create({a: 1}) } # install optimized wrapper

    assert_equal [{x: 1}, {}], instance.create({x: 1})
    assert_equal [{x: 1}, {y: 2}], instance.create({x: 1}, {y: 2})
  end

  it "forwards keywords for an all-keyword method through a sig wrapper" do
    klass = Class.new do
      extend T::Sig
      sig { params(livemode: T::Boolean, account: T.nilable(String)).returns(T::Array[T.untyped]).checked(:always) }
      def create(livemode:, account: nil)
        [livemode, account]
      end
    end

    instance = klass.new
    3.times { instance.create(livemode: true) } # install optimized wrapper

    assert_equal [false, "acct_1"], instance.create(livemode: false, account: "acct_1")
    opts = {livemode: true, account: "a"}
    assert_equal [true, "a"], instance.create(**opts)
  end

  # Regression: the wrapper-shape decision must read the parameters of the method being wrapped, not
  # `method_sig.parameters`. They diverge under the mocha pattern -- stub a method as
  # `def m(*args, **kwargs, &blk)`, then re-wrap it against the original fixed-arity sig via
  # `T::Utils.wrap_method_with_call_validation_if_needed`. Keying off the original sig's parameters picks
  # a fixed-arity wrapper for an `*args, **kwargs` method, dropping the ruby2_keywords flag so a trailing
  # `key: val` forwards positionally, breaking `.with { |t, **kw| }` matchers.
  it "preserves keyword forwarding when re-wrapping a splat method with a fixed-arity sig (mocha stub shape)" do
    klass = Class.new do
      extend T::Sig
      sig { params(name: String, opts: T::Hash[Symbol, T.untyped]).void.checked(:always) }
      def mutate(name, opts); end
    end
    original = klass.instance_method(:mutate)
    signature = T::Private::Methods.signature_for_method(original)

    # Emulate mocha: redefine as an `(*args, **kwargs)` capture, then re-wrap against the original sig.
    captured = nil
    klass.send(:define_method, :mutate) do |*args, **kwargs, &_blk|
      captured = {args: args, kwargs: kwargs}
    end
    stub = klass.instance_method(:mutate)
    wrapped = T::Utils.wrap_method_with_call_validation_if_needed(klass, signature, stub)

    # `wrap_method_with_call_validation_if_needed` is public API: it must return the
    # (possibly re-wrapped) UnboundMethod, not nil/void.
    assert_instance_of(UnboundMethod, wrapped)
    assert_equal(:mutate, wrapped.name)

    klass.new.mutate("rec_1", some_opt: :value)

    assert_equal(["rec_1"], captured[:args],
                 "the trailing keyword must not forward into the positional args")
    assert_equal({some_opt: :value}, captured[:kwargs],
                 "the trailing keyword must arrive as keywords (ruby2_keywords preserved)")
  end
end
