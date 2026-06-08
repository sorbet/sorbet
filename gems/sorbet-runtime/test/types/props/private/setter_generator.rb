# frozen_string_literal: true
require_relative '../../../test_helper'

class Opus::Types::Test::Props::Private::SetterGeneratorTest < Critic::Unit::UnitTest
  SetterGenerator = T::Props::Private::SetterGenerator

  private def own_slot_path(cls, name)
    SetterGenerator.own_slot_method(cls, name)&.source_location&.first
  end

  private def assert_stub(cls, name)
    assert_equal(SetterGenerator::STUB_DEF_PATH, own_slot_path(cls, name))
  end

  private def assert_compiled(cls, name)
    path = own_slot_path(cls, name)
    refute_nil(path)
    assert(path.start_with?(SetterGenerator::GENERATOR_FILE) && path.include?('(compiled:'),
      "expected compiled setter, got #{path}")
  end

  private def assert_interpreted_bmethod(cls, name)
    path = own_slot_path(cls, name)
    assert_includes(path.to_s, 'setter_factory.rb')
  end

  private def make_struct(&blk)
    Class.new(T::Struct, &blk)
  end

  describe 'compilation on first call' do
    it 'replaces the stub with a compiled plain def and preserves values and return values' do
      cls = make_struct do
        prop :simple, Integer
        prop :nilable, T.nilable(Integer)
        prop :complex, T::Array[Integer]
      end
      obj = cls.new(simple: 1, complex: [])

      assert_stub(cls, :simple=)
      obj.simple = 2
      assert_compiled(cls, :simple=)
      assert_equal(2, obj.simple)

      # __send__ return value parity with the interpreted setter_proc
      assert_equal(7, obj.__send__(:simple=, 7))
      assert_nil(obj.__send__(:nilable=, nil))
      assert_compiled(cls, :nilable=)
      assert_equal(5, obj.__send__(:nilable=, 5))
      assert_equal(5, obj.nilable)

      obj.complex = [1, 2]
      assert_compiled(cls, :complex=)
      assert_equal([1, 2], obj.complex)
    end

    it 'raises the same TypeError text as the interpreted setter' do
      compiled = make_struct { prop :x, Integer }
      # A hyphenated prop name is ineligible for compilation, so this class
      # keeps the interpreted bmethod path forever.
      interp = make_struct { prop :"x-i", Integer, without_accessors: true }
      interp.send(:define_method, :xi=, &interp.decorator.prop_rules(:"x-i").fetch(:setter_proc))

      cobj = compiled.new(x: 1)
      cobj.x = 2 # compile

      cerr = assert_raises(TypeError) { cobj.x = 'nope' }
      ierr = assert_raises(TypeError) { interp.allocate.xi = 'nope' }
      # Same message shape: only the prop name differs
      assert_equal(
        ierr.message.gsub('x-i', 'x').gsub(/Caller: .*/, ''),
        cerr.message.gsub(/Caller: .*/, '')
      )
      # The compiled frame is never reported as the caller
      refute_includes(cerr.message, '(compiled:')
    end

    it 'enforces setter arity like the interpreted setter' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      assert_raises(ArgumentError) { obj.__send__(:x=) }      # stub
      obj.x = 2                                               # compile
      assert_raises(ArgumentError) { obj.__send__(:x=) }      # compiled def
      assert_raises(ArgumentError) { obj.__send__(:x=, 1, 2) }
    end
  end

  describe 'private constant and anonymous prop types (case file [D])' do
    module Hidden
      class Secret
        def initialize(tag)
          @tag = tag
        end
        attr_reader :tag
      end
      private_constant :Secret

      def self.build(tag)
        Secret.new(tag)
      end

      def self.secret_class
        Secret
      end
    end

    it 'compiles setters whose prop type is a private constant' do
      cls = make_struct do
        prop :secret, Hidden.secret_class
        prop :maybe_secret, T.nilable(Hidden.secret_class)
        prop :secrets, T::Array[Hidden.secret_class]
      end
      obj = cls.new(secret: Hidden.build(1), secrets: [])

      obj.secret = Hidden.build(2)
      assert_compiled(cls, :secret=)
      assert_equal(2, obj.secret.tag)

      obj.maybe_secret = Hidden.build(3)
      assert_compiled(cls, :maybe_secret=)
      obj.maybe_secret = nil
      assert_nil(obj.maybe_secret)

      obj.secrets = [Hidden.build(4)]
      assert_compiled(cls, :secrets=)

      assert_raises(TypeError) { obj.secret = 'not a secret' }
      assert_raises(TypeError) { obj.secrets = ['not a secret'] }
    end

    it 'compiles setters whose prop type is an anonymous class with no constant path' do
      anon = Class.new
      cls = make_struct { prop :thing, anon }
      obj = cls.new(thing: anon.new)
      obj.thing = anon.new
      assert_compiled(cls, :thing=)
      assert_raises(TypeError) { obj.thing = 42 }
    end
  end

  describe 'setter_validate through the compiled path' do
    it 'is called with exactly (prop, value), only on type-check success, and never for nil' do
      calls = []
      cls = make_struct do
        prop :checked, T.nilable(Integer), setter_validate: ->(prop, value) { calls << [prop, value] }
      end
      obj = cls.new

      obj.checked = 1 # compiles
      assert_compiled(cls, :checked=)
      assert_equal([[:checked, 1]], calls)

      obj.checked = 2
      assert_equal([[:checked, 1], [:checked, 2]], calls)

      obj.checked = nil
      assert_equal(2, calls.length, 'validate must not run for nil')

      T::Configuration.call_validation_error_handler = proc {} # soft: don't raise
      begin
        obj.checked = 'wrong type'
        assert_equal(2, calls.length, 'validate must not run when the type check fails')
        assert_equal('wrong type', obj.checked, 'soft type errors still set the value')
      ensure
        T::Configuration.call_validation_error_handler = nil
      end
    end

    it 'propagates validate errors without setting the value' do
      cls = make_struct do
        prop :guarded, Integer, setter_validate: ->(_prop, value) { raise ArgumentError, 'invalid' if value > 10 }
      end
      obj = cls.new(guarded: 1)
      obj.guarded = 2
      assert_compiled(cls, :guarded=)
      err = assert_raises(ArgumentError) { obj.guarded = 11 }
      assert_equal('invalid', err.message)
      assert_equal(2, obj.guarded, 'a raising validate must leave the previous value')
    end
  end

  describe 'set-after-soft-error quirk' do
    it 'still sets the ivar when a soft handler swallows the type error (all shapes)' do
      cls = make_struct do
        prop :simple, Integer
        prop :simple_nilable, T.nilable(Integer)
        prop :complex, T::Array[Integer]
        prop :complex_nilable, T.nilable(T::Array[Integer])
      end
      obj = cls.new(simple: 1, complex: [])
      # compile all four
      obj.simple = 2
      obj.simple_nilable = 2
      obj.complex = [2]
      obj.complex_nilable = [2]

      T::Configuration.call_validation_error_handler = proc {}
      begin
        obj.simple = 'bad'
        obj.simple_nilable = 'bad'
        obj.complex = 'bad'
        obj.complex_nilable = 'bad'
        assert_equal('bad', obj.simple)
        assert_equal('bad', obj.simple_nilable)
        assert_equal('bad', obj.complex)
        assert_equal('bad', obj.complex_nilable)
      ensure
        T::Configuration.call_validation_error_handler = nil
      end
    end

    it 'reports errors against the defining class, not the receiver class' do
      parent = Class.new(T::InexactStruct) { prop :x, Integer }
      def parent.name
        'ParentClassForErrors'
      end
      child = Class.new(parent)
      obj = child.new(x: 1)
      obj.x = 2
      assert_compiled(parent, :x=)
      err = assert_raises(TypeError) { obj.x = 'bad' }
      assert_includes(err.message, "Can't set ParentClassForErrors.x")
    end
  end

  describe 'visibility' do
    it 'preserves a privatized setter through compilation' do
      cls = make_struct { prop :token, String }
      cls.send(:private, :token=)
      obj = cls.new(token: 'a')
      obj.__send__(:token=, 'b') # first call compiles
      assert_compiled(cls, :token=)
      assert(cls.private_method_defined?(:token=), 'setter must stay private after compilation')
      assert_raises(NoMethodError) { obj.token = 'c' }
      assert_equal('b', obj.token)
    end
  end

  describe 'pending sig declarations (method_added hook)' do
    it 'does not bind a pending sig to a setter compiled mid-class-body, and the setter keeps working' do
      cls = Class.new(T::Struct) do
        extend T::Sig
        prop :x, Integer
        sig { returns(String) }
        new(x: 1).x = 2 # first call: compiles the setter with a sig pending
        def foo
          42
        end
      end
      obj = cls.new(x: 1)
      # The setter must not be bricked by the pending declaration ...
      obj.x = 3
      assert_equal(3, obj.x)
      obj.x = 4
      assert_equal(4, obj.x)
      assert_compiled(cls, :x=)
      # ... must not have stolen the sig ...
      assert_nil(T::Private::Methods.signature_for_method(cls.instance_method(:x=)))
      # ... and the sig must have bound to the next real def, with enforcement.
      assert_raises(TypeError) { obj.foo }
    ensure
      T::Private::DeclState.current.reset!
    end

    it 'does not drop a pending sig on another module when a first-call compile happens (cross-module)' do
      other = Class.new do
        extend T::Sig
      end
      k = make_struct { prop :x, Integer }
      obj = k.new(x: 1)

      # Open a sig declaration on `other`, then trigger a first-call compile
      # on `k` before the sig's method is defined.
      other.class_eval do
        sig { returns(String) }
      end
      obj.x = 2 # compiles; must not fire method_added with the foreign declaration pending
      assert_compiled(k, :x=)
      other.class_eval do
        def bar
          42
        end
      end

      refute_nil(T::Private::Methods.signature_for_method(other.instance_method(:bar)),
        'the pending sig must still bind to the next real def')
      assert_raises(TypeError) { other.new.bar }
      obj.x = 3
      assert_equal(3, obj.x)
    ensure
      T::Private::DeclState.current.reset!
    end
  end

  describe 'stale stub handles (case file [A])' do
    it 'does not clobber a manual setter redefinition via a pre-call Method handle' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      handle = obj.method(:x=) # stale handle to the stub
      cls.class_eval do
        def x=(val)
          @x = val.to_i * 100
        end
      end

      handle.call(3) # must neither clobber the user's def nor raise
      assert_equal(3, obj.x, 'a stale handle keeps its own (validated, interpreted) semantics')
      obj.x = 4
      assert_equal(400, obj.x, 'the manual redefinition must stay live')
      handle.call(5) # stays safe on later stale calls too
      assert_equal(5, obj.x)
      assert_raises(TypeError) { handle.call('bad') }
      obj.x = 6
      assert_equal(600, obj.x)

      # The queue entry must never compile over the user's definition.
      refute_equal(SetterGenerator::STUB_DEF_PATH, own_slot_path(cls, :x=))
      refute_includes(own_slot_path(cls, :x=).to_s, '(compiled:')
    end

    it 'does not resurrect a removed setter via a pre-call Method handle' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      handle = obj.method(:x=)
      cls.send(:remove_method, :x=)

      handle.call(3)
      assert_equal(3, obj.x, 'the stale handle still validates and writes, like a stale bmethod at base')
      assert_raises(TypeError) { handle.call('bad') }
      assert_nil(SetterGenerator.own_slot_method(cls, :x=), 'nothing may be re-defined')
      refute(cls.method_defined?(:x=))
    end
  end

  describe 'Class#dup and Class#clone' do
    it 'dup taken before the first call: setters keep working (interpreted) on the copy' do
      cls = make_struct { prop :z, Integer }
      copy = cls.dup

      obj = copy.new(z: 1)
      obj.z = 2
      assert_equal(2, obj.z)
      10.times { |i| obj.z = i }
      assert_equal(9, obj.z)
      assert_raises(TypeError) { obj.z = 'bad' }

      # The original still works and validates (its first call may already
      # have been compiled via the copy's stub).
      orig_obj = cls.new(z: 1)
      orig_obj.z = 5
      assert_equal(5, orig_obj.z)
      assert_raises(TypeError) { orig_obj.z = 'bad' }
    end

    it 'clone taken before the first call: setters keep working on the copy' do
      cls = make_struct { prop :z, Integer }
      copy = cls.clone

      obj = copy.new(z: 1)
      3.times { |i| obj.z = i }
      assert_equal(2, obj.z)
      assert_raises(TypeError) { obj.z = 'bad' }
    end

    it 'dup taken after compilation copies a working compiled setter' do
      cls = make_struct { prop :z, Integer }
      o = cls.new(z: 1)
      o.z = 2 # compile on the original
      assert_compiled(cls, :z=)

      copy = cls.dup
      obj = copy.new(z: 1)
      3.times { |i| obj.z = i }
      assert_equal(2, obj.z)
      assert_raises(TypeError) { obj.z = 'bad' }
    end
  end

  describe 'fallbacks' do
    it 'runs interpreted forever on a frozen class without consuming the queue entry' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      cls.freeze
      3.times { |i| obj.x = i }
      assert_equal(2, obj.x)
      assert_equal(9, obj.__send__(:x=, 9))
      assert_stub(cls, :x=)
      assert_raises(TypeError) { obj.x = 'bad' }
    end

    it 'falls back to the interpreted setter for ineligible prop names' do
      cls = make_struct do
        prop :if, T.nilable(Integer) # reserved word
      end
      obj = cls.new
      3.times { |i| obj.__send__(:if=, i) }
      assert_equal(2, obj.if)
      assert_interpreted_bmethod(cls, :if=)
      assert_raises(TypeError) { obj.__send__(:if=, 'bad') }
    end

    it 'permanently falls back when a foreign constant occupies the container name, without clobbering it' do
      logged = []
      T::Configuration.log_info_handler = proc { |msg, _extra| logged << msg }
      begin
        cls = make_struct { prop :x, Integer }
        cls.const_set(:SORBET_RT_PROPS_x_G1, :user_constant)
        obj = cls.new(x: 1)
        3.times { |i| obj.x = i }
        assert_equal(2, obj.x)
        assert_interpreted_bmethod(cls, :x=)
        assert_equal(:user_constant, cls.const_get(:SORBET_RT_PROPS_x_G1))
        assert_equal(1, logged.grep(/falling back to the interpreted setter/).length)
        assert_raises(TypeError) { obj.x = 'bad' }
      ensure
        T::Configuration.log_info_handler = nil
      end
    end

    it 'installs a permanent interpreted setter when lazy evaluation is disabled' do
      T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
      begin
        cls = make_struct { prop :x, Integer }
        obj = cls.new(x: 1)
        assert_stub(cls, :x=)
        obj.x = 2
        assert_interpreted_bmethod(cls, :x=)
        obj.x = 3
        assert_equal(3, obj.x)
        assert_raises(TypeError) { obj.x = 'bad' }
      ensure
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end

    it 'runs interpreted without compiling while another claim is in flight, then compiles' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      entry = cls.decorator.instance_variable_get(:@specialized_methods).fetch(:x=)

      entry.state = :compiling # simulate another thread mid-compile
      obj.x = 2
      assert_equal(2, obj.x)
      assert_stub(cls, :x=)

      entry.state = :pending
      obj.x = 3
      assert_compiled(cls, :x=)
      assert_equal(3, obj.x)
    end

    it 'runs interpreted when the lock is owned by the current thread (trap re-entrancy guard)' do
      cls = make_struct { prop :x, Integer }
      obj = cls.new(x: 1)
      lock = cls.decorator.instance_variable_get(:@specialization_lock)
      lock.lock
      begin
        obj.x = 2
        assert_equal(2, obj.x)
        assert_stub(cls, :x=)
      ensure
        lock.unlock
      end
      obj.x = 3
      assert_compiled(cls, :x=)
    end

    it 'works from a signal trap handler and compiles on a later normal call' do
      skip 'no signal support' unless Signal.list.key?('USR1')
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      fired = false
      previous = Signal.trap('USR1') do
        obj.x = 42
        fired = true
      end
      begin
        Process.kill('USR1', Process.pid)
        500.times do
          break if fired
          sleep(0.01)
        end
        assert(fired, 'trap handler did not run')
        assert_equal(42, obj.x)
        assert_stub(cls, :x=) # trap context cannot compile
        obj.x = 43
        assert_compiled(cls, :x=)
        assert_equal(43, obj.x)
      ensure
        Signal.trap('USR1', previous || 'DEFAULT')
      end
    end
  end

  describe 'eagerly_specialize_prop_methods!' do
    it 'compiles queued setters without any call' do
      cls = make_struct { prop :x, Integer }
      assert_stub(cls, :x=)
      cls.decorator.eagerly_specialize_prop_methods!
      assert_compiled(cls, :x=)
      obj = cls.new(x: 1)
      obj.x = 2
      assert_equal(2, obj.x)
      assert_empty(cls.decorator.instance_variable_get(:@specialized_methods))
    end

    it 'still compiles after disable_lazy_evaluation! (explicit-eager exemption)' do
      T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
      begin
        cls = make_struct { prop :x, Integer }
        cls.decorator.eagerly_specialize_prop_methods!
        assert_compiled(cls, :x=)
        obj = cls.new(x: 1)
        obj.x = 2
        assert_equal(2, obj.x)
      ensure
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end

    it 'drops queue entries for setters the user redefined, instead of clobbering them' do
      cls = make_struct { prop :x, Integer }
      cls.class_eval do
        def x=(val)
          @x = val * 10
        end
      end
      cls.decorator.eagerly_specialize_prop_methods!
      obj = cls.new(x: 1)
      obj.x = 2
      assert_equal(20, obj.x, 'user redefinition must survive the eager drain')
      assert_empty(cls.decorator.instance_variable_get(:@specialized_methods))
    end

    it 'is not required for the hardened recipe: eager + disable falls back safely' do
      cls = make_struct { prop :x, Integer }
      cls.decorator.eagerly_define_lazy_methods!
      assert_stub(cls, :x=)
      T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!
      begin
        obj = cls.new(x: 1)
        obj.x = 2 # no eval allowed: must install the interpreted setter
        assert_interpreted_bmethod(cls, :x=)
        assert_equal(2, obj.x)
        obj.x = 3
        assert_equal(3, obj.x)
      ensure
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end
  end

  describe 'prop redefinition' do
    it 'enforces the new type when a prop is redefined before any call' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      cls.class_eval { prop :x, T.nilable(String), override: true }
      obj = cls.new
      obj.x = 'str'
      assert_compiled(cls, :x=)
      assert_equal('str', obj.x)
      assert_raises(TypeError) { obj.x = 5 }
    end

    it 'enforces the new type when a prop is redefined after compilation' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      obj.x = 5 # compile generation 1
      assert_compiled(cls, :x=)
      cls.class_eval { prop :x, T.nilable(String), override: true }
      assert_stub(cls, :x=)
      obj.x = 'str' # compile generation 2
      assert_compiled(cls, :x=)
      assert_equal('str', obj.x)
      assert_raises(TypeError) { obj.x = 5 }
    end
  end

  describe 'stub-over-definition torn states' do
    it 'never raises when the live definition is a stub with an empty queue, and re-arms compilation' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      decorator = cls.decorator
      lock = decorator.instance_variable_get(:@specialization_lock)
      # Simulate the torn outcome of a racing prop redefinition (the
      # enqueue-then-install window): the queue entry was consumed but a
      # stub is the live own-slot definition.
      lock.synchronize { decorator.instance_variable_get(:@specialized_methods).delete(:x=) }
      assert_stub(cls, :x=)

      obj.x = 5 # must not raise; runs interpreted and re-arms
      assert_equal(5, obj.x)
      refute_empty(decorator.instance_variable_get(:@specialized_methods),
        'the convergence arm must re-enqueue so a later call can compile')

      obj.x = 6 # the re-armed entry compiles now
      assert_compiled(cls, :x=)
      assert_equal(6, obj.x)
      assert_raises(TypeError) { obj.x = 'bad' }
    end
  end

  describe 'inheritance and prepend' do
    it 'compiles on the defining class when first called through a subclass instance' do
      parent = Class.new(T::InexactStruct) { prop :x, T.nilable(Integer) }
      child = Class.new(parent)
      cobj = child.new
      cobj.x = 1
      assert_compiled(parent, :x=)
      assert_nil(SetterGenerator.own_slot_method(child, :x=), 'child must inherit, not copy')
      pobj = parent.new
      pobj.x = 2
      assert_equal(2, pobj.x)
    end

    it 'runs a prepended wrapper exactly once per call, including the compiling call' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      hook_calls = 0
      wrapper = Module.new do
        define_method(:x=) do |val|
          hook_calls += 1
          super(val + 1)
        end
      end
      cls.prepend(wrapper)
      obj = cls.new

      obj.x = 10 # first call: stub fires under the prepended wrapper
      assert_equal(11, obj.x)
      assert_equal(1, hook_calls, 'prepended wrapper must run exactly once on the compiling call')
      assert_compiled(cls, :x=)

      obj.x = 20
      assert_equal(21, obj.x)
      assert_equal(2, hook_calls)
    end
  end

  describe 'concurrency' do
    it 'survives many threads racing the first call' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      barrier = Queue.new
      errs = Queue.new
      threads = Array.new(8) do |i|
        Thread.new do
          barrier.pop
          100.times { obj.x = i }
        rescue => e
          errs << e
        end
      end
      8.times { barrier << true }
      threads.each(&:join)
      assert_empty(errs.size.times.map { errs.pop })
      assert_compiled(cls, :x=)
      assert_kind_of(Integer, obj.x)
    end

    it 'lets a prop redefinition win over an in-flight first-call compile' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      decorator = cls.decorator
      entry = decorator.instance_variable_get(:@specialized_methods).fetch(:x=)

      # Make the gen-1 compile block until we let it proceed, so we can
      # interleave a prop redefinition with an in-flight first call.
      started = Queue.new
      gate = Queue.new
      original = SetterGenerator.method(:define_compiled_setter!)
      SetterGenerator.define_singleton_method(:define_compiled_setter!) do |*args|
        if args[1] == :x && args[3] == entry.gen
          started << true
          gate.pop
        end
        original.call(*args)
      end
      begin
        compiling_call = Thread.new { obj.x = 5 }
        started.pop
        # Redefinition re-enqueues a new entry and reinstalls the stub while
        # the gen-1 compile is still in flight.
        cls.class_eval { prop :x, T.nilable(String), override: true }
        gate << true
        compiling_call.join
      ensure
        SetterGenerator.define_singleton_method(:define_compiled_setter!, original)
      end

      # The in-flight call ran with the old (Integer) semantics.
      assert_equal(5, obj.x)
      # The compiler detected the re-enqueue and put the new stub back.
      assert_stub(cls, :x=)
      refute_empty(decorator.instance_variable_get(:@specialized_methods))
      # The redefined prop wins from here on.
      obj.x = 'str'
      assert_compiled(cls, :x=)
      assert_equal('str', obj.x)
      assert_raises(TypeError) { obj.x = 5 }
    end

    it 'repairs a stale-generation publish that lands after a redefined prop already compiled' do
      cls = make_struct { prop :x, T.nilable(Integer) }
      obj = cls.new
      decorator = cls.decorator
      entry1 = decorator.instance_variable_get(:@specialized_methods).fetch(:x=)

      started = Queue.new
      gate = Queue.new
      original = SetterGenerator.method(:define_compiled_setter!)
      SetterGenerator.define_singleton_method(:define_compiled_setter!) do |*args|
        if args[1] == :x && args[3] == entry1.gen
          started << true
          gate.pop
        end
        original.call(*args)
      end
      begin
        stale_call = Thread.new { obj.x = 5 }
        started.pop
        # While the gen-1 compile is gated: redefine the prop, then let a
        # fresh call fully compile, publish, and DEQUEUE the new generation.
        cls.class_eval { prop :x, T.nilable(String), override: true }
        obj.x = 'fresh'
        assert_compiled(cls, :x=)
        assert_empty(decorator.instance_variable_get(:@specialized_methods))
        # Now release gen-1: it publishes a STALE definition over the fresh
        # one and must detect that at the final sync, re-enqueue, and
        # reinstall a stub -- never silently keep enforcing the old type.
        gate << true
        stale_call.join
      ensure
        SetterGenerator.define_singleton_method(:define_compiled_setter!, original)
      end

      # The torn call ran with its own (old) generation's semantics.
      assert_equal(5, obj.x)
      # The stale publish was detected and compilation re-armed.
      assert_stub(cls, :x=)
      refute_empty(decorator.instance_variable_get(:@specialized_methods))
      # The redefined prop's semantics win from here on.
      obj.x = 'str'
      assert_compiled(cls, :x=)
      assert_equal('str', obj.x)
      assert_raises(TypeError) { obj.x = 5 }
    end
  end
end
