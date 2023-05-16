# frozen_string_literal: true
require_relative '../test_helper'
require 'concurrent/atomic/cyclic_barrier'

class Opus::Types::Test::EdgeCasesTest < Critic::Unit::UnitTest
  it 'can type ==' do
    klass = Class.new do
      extend T::Sig
      extend T::Helpers
      sig {override.params(other: T.self_type).returns(T::Boolean)}
      def ==(other)
        true
      end
    end
    assert_equal(klass.new, klass.new)
  end

  it 'handles attr_writer and attr_accessor generated writers' do
    klass = Class.new do
      extend T::Sig
      extend T::Helpers
      sig {params(foo: String).returns(String)}
      attr_writer :foo
      sig {params(bar: Integer).returns(Integer)}
      attr_accessor :bar
    end

    assert_equal("foo", klass.new.foo = "foo")
    err = assert_raises(TypeError) do
      klass.new.foo = 42
    end
    assert_match(/Expected type String, got type Integer/, err.message)
    assert_equal(42, klass.new.bar = 42)
    # TODO: This should also raise a type error, but currently doesn't because
    # the sig only affects the reader part of the attr_accessor, not the writer.
    assert_equal("foo", klass.new.bar = "foo")
  end

  it 'handles module_function on including class' do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      sig {params(foo: String).returns(String)}
      module_function def foo(foo)
        foo
      end
    end

    klass = Class.new do
      include mod
    end

    assert_equal('bar', klass.new.send(:foo, 'bar'))
    assert_equal('bar', mod.foo('bar'))
  end

  private def counting_allocations
    before = GC.stat[:total_allocated_objects]
    yield
    GC.stat[:total_allocated_objects] - before - 1 # Subtract one for the allocation by GC.stat itself
  end

  private def check_alloc_counts
    @check_alloc_counts = Gem::Version.new(RUBY_VERSION) < Gem::Version.new('3.0')
  end

  describe 'aliasing' do
    describe 'instance method' do
      it 'handles alias_method with runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol)}
          def foo(x=:foo)
            x
          end
          alias_method :bar, :foo
        end
        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Should still validate
        assert_raises(TypeError) do
          klass.new.bar(1)
        end

        # Should use fast path
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias_method without runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def foo(x=:foo)
            x
          end
          alias_method :bar, :foo
        end
        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Shouldn't add overhead
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias with runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol)}
          def foo(x=:foo)
            x
          end
          alias :bar :foo
        end
        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Should still validate
        assert_raises(TypeError) do
          klass.new.bar(1)
        end

        # Should use fast path
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias without runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def foo(x=:foo)
            x
          end
          alias :bar :foo
        end
        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Shouldn't add overhead
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias to superclass method with runtime checking' do
        superclass = Class.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol)}
          def foo(x=:foo)
            x
          end
        end

        subclass = Class.new(superclass) do
          alias_method :bar, :foo
        end

        assert_equal(:foo, subclass.new.foo)
        assert_equal(:foo, subclass.new.bar)
        assert_equal(:foo, superclass.new.foo)

        # Should still validate
        assert_raises(TypeError) do
          subclass.new.bar(1)
        end

        # Should use fast path
        obj = subclass.new
        allocs = counting_allocations {obj.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias to superclass method without runtime checking' do
        superclass = Class.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def foo(x=:foo)
            x
          end
        end

        subclass = Class.new(superclass) do
          alias_method :bar, :foo
        end

        assert_equal(:foo, subclass.new.foo)
        assert_equal(:foo, subclass.new.bar)
        assert_equal(:foo, superclass.new.foo)

        # Shouldn't add overhead
        obj = subclass.new
        allocs = counting_allocations {obj.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias_method to included method with runtime checking' do
        mod = Module.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol)}
          def foo(x=:foo)
            x
          end
        end

        klass = Class.new do
          include mod
          alias_method :bar, :foo
        end

        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Should still validate
        assert_raises(TypeError) do
          klass.new.bar(1)
        end

        # Should use fast path
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias_method to included method without runtime checking' do
        mod = Module.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def foo(x=:foo)
            x
          end
        end

        klass = Class.new do
          include mod
          alias_method :bar, :foo
        end

        assert_equal(:foo, klass.new.foo)
        assert_equal(:foo, klass.new.bar)

        # Shouldn't add overhead
        obj = klass.new
        allocs = counting_allocations {obj.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end
    end

    describe 'singleton method' do
      it 'handles alias_method with runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol)}
          def self.foo(x=:foo)
            x
          end
          class << self
            alias_method :bar, :foo
          end
        end
        assert_equal(:foo, klass.bar)
        assert_equal(:foo, klass.foo)

        # Should still validate
        assert_raises(TypeError) do
          klass.bar(1)
        end

        # Should use fast path
        allocs = counting_allocations {klass.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias_method without runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {returns(Symbol).checked(:never)}
          def self.foo
            :foo
          end
          class << self
            alias_method :bar, :foo
          end
        end
        assert_equal(:foo, klass.bar)
        assert_equal(:foo, klass.foo)

        # Shouldn't add overhead
        klass.bar # Need extra call since first one came before `foo` unwrap
        allocs = counting_allocations {klass.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias with runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {params(x: Symbol).returns(Symbol)}
          def self.foo(x=:foo)
            x
          end
          class << self
            alias :bar :foo
          end
        end
        assert_equal(:foo, klass.bar)
        assert_equal(:foo, klass.foo)

        # Should still validate
        assert_raises(TypeError) do
          klass.bar(1)
        end

        # Should use fast path
        allocs = counting_allocations {klass.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias without runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          sig {returns(Symbol).checked(:never)}
          def self.foo
            :foo
          end
          class << self
            alias :bar :foo
          end
        end
        assert_equal(:foo, klass.bar)
        assert_equal(:foo, klass.foo)

        # Shouldn't add overhead
        klass.bar # Need extra call since first one came before `foo` unwrap
        allocs = counting_allocations {klass.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias_method to superclass method with runtime checking' do
        superclass = Class.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol)}
          def self.foo(x=:foo)
            x
          end
        end

        subclass = Class.new(superclass) do
          class << self
            alias_method :bar, :foo
          end
        end

        assert_equal(:foo, subclass.foo)
        assert_equal(:foo, subclass.bar)
        assert_equal(:foo, superclass.foo)

        # Should still validate
        assert_raises(TypeError) do
          subclass.bar(1)
        end

        # Should use fast path
        allocs = counting_allocations {subclass.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias_method to superclass method without runtime checking' do
        superclass = Class.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def self.foo(x=:foo)
            x
          end
        end

        subclass = Class.new(superclass) do
          class << self
            alias_method :bar, :foo
          end
        end

        assert_equal(:foo, subclass.foo)
        assert_equal(:foo, subclass.bar)
        assert_equal(:foo, superclass.foo)

        # Shouldn't add overhead
        allocs = counting_allocations {subclass.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles alias_method to extended method with runtime checking' do
        mod = Module.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol)}
          def foo(x=:foo)
            x
          end
        end

        klass = Class.new do
          extend mod

          class << self
            alias_method :bar, :foo
          end
        end

        assert_equal(:foo, klass.foo)
        assert_equal(:foo, klass.bar)

        # Should still validate
        assert_raises(TypeError) do
          klass.bar(1)
        end

        # Should use fast path
        allocs = counting_allocations {klass.bar}
        assert_operator(allocs, :<, 5) if check_alloc_counts
      end

      it 'handles alias_method to extended method without runtime checking' do
        mod = Module.new do
          extend T::Sig

          sig {params(x: Symbol).returns(Symbol).checked(:never)}
          def foo(x=:foo)
            x
          end
        end

        klass = Class.new do
          extend mod

          class << self
            alias_method :bar, :foo
          end
        end

        assert_equal(:foo, klass.foo)
        assert_equal(:foo, klass.bar)

        # Shouldn't add overhead
        allocs = counting_allocations {klass.bar}
        assert_equal(0, allocs) if check_alloc_counts
      end

      it 'handles method reference without sig' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          def self.foo(arr)
            arr.map(&method(:bar))
          end
          def self.bar(x)
            -x
          end
        end
        assert_equal([1, 2, 3], klass.foo([-1, -2, -3]))
      end

      it 'handles method reference with runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          def self.foo(arr)
            arr.map(&method(:bar))
          end

          sig {params(x: Integer).returns(Integer)}
          def self.bar(x)
            -x
          end
        end
        assert_equal([1, 2, 3], klass.foo([-1, -2, -3]))
        assert_raises(TypeError) do
          klass.foo(["-1", "-2", "-3"])
        end
      end

      it 'handles method reference without runtime checking' do
        klass = Class.new do
          extend T::Sig
          extend T::Helpers
          def self.foo(arr)
            arr.map(&method(:bar))
          end

          sig {params(x: Integer).returns(Integer).checked(:never)}
          def self.bar(x)
            -x
          end
        end
        assert_equal([1, 2, 3], klass.foo([-1, -2, -3]))
        assert_equal(["-1", "-2", "-3"], klass.foo(["-1", "-2", "-3"]))
      end
    end
  end

  it 'works for any_instance' do
    klass = Class.new do
      extend T::Sig
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
      extend T::Sig
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
      extend T::Sig
      extend T::Helpers
      sig {overridable.returns(Symbol)}
      def self.foo
        :parent
      end
    end
    child = Class.new(parent) do
      extend T::Sig
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
      extend T::Sig
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
      extend T::Sig
      extend T::Helpers
      abstract!

      sig {abstract.void}
      private def foo; end
    end
    T::Private::Abstract::Validate.validate_abstract_module(klass)
  end

  it 'handles class scope change when already hooked' do
    klass = Class.new do
      extend T::Sig
      extend T::Helpers
      sig {returns(Symbol)}
      def foo
        :foo
      end

      class << self
        extend T::Sig
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

  it 'handles class scope change when hooked from class << self' do
    klass = Class.new do
      class << self
        extend T::Sig

        sig {returns(Symbol)}
        def foo
          :foo
        end
      end

      extend T::Sig
      sig {returns(Symbol)}
      def bar
        :bar
      end
    end
    assert_equal(:foo, klass.foo)
    assert_equal(:bar, klass.new.bar)
  end

  it 'checks for type error in class << self' do
    klass = Class.new do
      class << self
        extend T::Sig

        sig {returns(Symbol)}
        def bad_return
          1
        end
      end
    end

    assert_raises(TypeError) do
      klass.bad_return
    end
  end

  it 'checks for type error for self.foo in class << self' do
    klass = Class.new do
      class << self
        extend T::Sig

        sig {returns(Symbol)}
        def self.bad_return
          1
        end

        def sanity; end
      end
    end

    klass.sanity
    assert_raises(TypeError) do
      klass.singleton_class.bad_return
    end
  end

  it "calls a user-defined singleton_method_added when registering hooks" do
    klass = Class.new do
      class << self
        def singleton_method_added(name)
          super
          @called ||= []
          @called << name
        end
      end

      extend T::Sig
      sig {returns(Symbol)}
      def foo; end

      def self.post_hook; end
    end

    assert_equal(
      %i[
        singleton_method_added
        post_hook
      ],
      klass.instance_variable_get(:@called)
    )
  end

  it "allows custom hooks" do
    parent = Class.new do
      extend T::Sig
      def self.method_added(method)
        super(method)
      end
      def self.singleton_method_added(method)
        super(method)
      end
    end
    Class.new(parent) do
      extend T::Sig
      sig {void}
      def a; end
      sig {void}
      def b; end
      sig {void}
      def c; end
    end
  end

  it "still calls our hooks if the user supers up" do
    c1 = Class.new do
      extend T::Sig
      sig {returns(Integer)}
      def foo
        1
      end
      def self.method_added(method)
        super(method)
      end
      def self.singleton_method_added(method)
        super(method)
      end
      sig {returns(Integer)}
      def bar
        "bad"
      end
    end
    assert_equal(1, c1.new.foo)
    assert_raises(TypeError) {c1.new.bar}
  end

  it "forbids adding a sig to method_added" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig {params(method: Symbol).void}
        def self.method_added(method)
          super(method)
        end
      end
    end
    assert_includes(err.message, "Putting a `sig` on `method_added` is not supported")
  end

  it "forbids adding a sig to singleton_method_added" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig {params(method: Symbol).void}
        def self.singleton_method_added(method)
          super(method)
        end
      end
    end
    assert_includes(err.message, "Putting a `sig` on `singleton_method_added` is not supported")
  end

  it "does not make sig available to attached class" do
    assert_raises(NoMethodError) do
      Class.new do
        class << self
          extend T::Sig
          sig {void}
          def jojo; end
        end

        sig {void} # this shouldn't work since sig is not available
        def self.post; end
      end
    end
  end

  it 'keeps raising for bad sigs' do
    klass = Class.new do
      extend T::Sig
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
      extend T::Sig
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

  it 'does not crash when two threads call the same wrapper' do
    begin
      barrier = Concurrent::CyclicBarrier.new(2)
      mutex = Mutex.new
      replaced = T::Private::ClassUtils.replace_method(T::Private::Methods.singleton_class, :run_sig_block_for_method) do |*args|
        barrier.wait
        mutex.synchronize {replaced.bind(T::Private::Methods).call(*args)}
      end

      klass = Class.new do
        extend T::Sig
        sig {void}
        def self.hello; end
      end

      thread = Thread.new {klass.hello}
      klass.hello
    ensure
      thread&.join
      replaced&.restore
    end
  end

  it 'redefines wrapper methods with the right visibility upfront' do
    method_redefinitions = []

    klass = Class.new do
      define_singleton_method(:method_added) do |name|
        # Reaching into a private method for testing purposes
        visibility = T::Private::ClassUtils.send(:visibility_method_name, self, name)

        method_redefinitions << [name, visibility]

        super(name)
      end

      extend T::Sig

      sig {returns(String)}
      def a_public_method
        "public"
      end

      protected

      sig {returns(String)}
      def a_protected_method
        "protected"
      end

      private

      sig {returns(String)}
      def a_private_method
        "private"
      end
    end

    instance = klass.new

    assert_equal("public", instance.a_public_method)
    assert_equal("protected", instance.send(:a_protected_method))
    assert_equal("private", instance.send(:a_private_method))

    # We are not interested in repeated method redefinitions
    unique_method_redefinitions = method_redefinitions.uniq

    assert_equal([
      %i[a_private_method private],
      %i[a_protected_method protected],
      %i[a_public_method public],
    ], unique_method_redefinitions.sort)
  end

  it "can mark a class abstract! even if it defines a method called method" do
    Class.new do
      extend T::Helpers
      # bad override of Object#method
      def self.method; end
      abstract!
    end
  end
end
