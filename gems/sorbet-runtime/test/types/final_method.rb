# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::FinalMethodTest < Critic::Unit::UnitTest
  before do
    T::Configuration.enable_final_checks_on_hooks
  end

  after do
    T::Private::DeclState.current.reset!
    T::Configuration.reset_final_checks_on_hooks
  end

  CLASS_REGEX_STR = "#<Class:0x[0-9a-f]+>"
  CLASS_CLASS_REGEX_STR = "#<Class:#<Class:0x[0-9a-f]+>>"
  MODULE_REGEX_STR = "#<Module:0x[0-9a-f]+>"

  private def assert_msg_matches(regex, final_line, method_line, explanation, err)
    lines = err.message.split("\n")
    assert_equal(3, lines.length)
    assert_match(regex, lines[0])
    assert_match(%r{Made final here: .*test.*/types/final_method.rb:#{final_line}}, lines[1])
    assert_match(%r{#{explanation} here: .*test.*/types/final_method.rb:#{method_line}}, lines[2])
  end

  private def assert_redefined_err(method_name, klass_str, final_line, method_line, err)
    regex = /The method `#{method_name}` on #{klass_str} was declared as final and cannot be redefined/
    assert_msg_matches(regex, final_line, method_line, "Redefined", err)
  end

  private def assert_overridden_err(method_name, klass_str, method_str, final_line, method_line, err)
    regex = /The method `#{method_name}` on #{klass_str} was declared as final and cannot be overridden in #{method_str}/
    assert_msg_matches(regex, final_line, method_line, "Overridden", err)
  end

  it "allows declaring an instance method as final" do
    Class.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
  end

  it "allows declaring a class method as final" do
    Class.new do
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids redefining a final instance method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        sig(:final) {void}
        def foo; end
      end
    end
    assert_redefined_err('foo', CLASS_REGEX_STR, __LINE__ - 5, __LINE__ - 3, err)
  end

  it "forbids redefining a final class method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        sig(:final) {void}
        def self.foo; end
      end
    end
    assert_redefined_err('foo', CLASS_CLASS_REGEX_STR, __LINE__ - 5, __LINE__ - 3, err)
  end

  it "forbids redefining a final instance method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        sig {void}
        def foo; end
      end
    end
    assert_redefined_err('foo', CLASS_REGEX_STR, __LINE__ - 5, __LINE__ - 3, err)
  end

  it "forbids redefining a final class method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        sig {void}
        def self.foo; end
      end
    end
    assert_redefined_err('foo', CLASS_CLASS_REGEX_STR, __LINE__ - 5, __LINE__ - 3, err)
  end

  it "forbids redefining a final instance method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:0x[0-9a-f]+> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final class method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:#<Class:0x[0-9a-f]+>> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a secretly-declared final instance method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        sig(:final) {void}
        def i_am_secretly_final; end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final instance method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        sig {void}
        def i_am_secretly_final; end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final instance method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        def i_am_secretly_final; end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 6, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final instance method with a secretly-declared final method" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 12, __LINE__ - 4, err)
  end

  it "forbids redefining a secretly-declared final instance method with a secretly-declared method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        built_sig = T::Private::Methods._declare_sig(self) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 12, __LINE__ - 4, err)
  end

  it "forbids redefining a secretly-declared final instance method with a secretly-declared method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def i_am_secretly_final; end
        end

        T::Private::Methods._with_declared_signature(self, nil) do
          def i_am_secretly_final; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final', CLASS_REGEX_STR, __LINE__ - 8, __LINE__ - 4, err)
  end

  it "forbids redefining a secretly-declared final class method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        sig(:final) {void}
        def self.i_am_secretly_final2; end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final class method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        sig {void}
        def self.i_am_secretly_final2; end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final class method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        def self.i_am_secretly_final2; end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 6, __LINE__ - 3, err)
  end

  it "forbids redefining a secretly-declared final class method with a secretly-declared final method" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 12, __LINE__ - 4, err)
  end

  it "forbids redefining a secretly-declared final class method with a secretly-declared method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        built_sig = T::Private::Methods._declare_sig(self) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 12, __LINE__ - 4, err)
  end

  it "forbids redefining a secretly-declared final class method with a secretly-declared method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        extend T::Helpers

        built_sig = T::Private::Methods._declare_sig(self, :final) do
          void
        end

        T::Private::Methods._with_declared_signature(self, built_sig) do
          def self.i_am_secretly_final2; end
        end

        T::Private::Methods._with_declared_signature(self, nil) do
          def self.i_am_secretly_final2; end
        end
      end
    end
    assert_redefined_err('i_am_secretly_final2', CLASS_CLASS_REGEX_STR, __LINE__ - 8, __LINE__ - 4, err)
  end

  it "forbids redefinition with .checked(:never)" do
    assert_raises(RuntimeError) do
      c = Class.new do
        extend T::Sig
        sig(:final) {void.checked(:never)}
        def self.foo; end
      end

      c.foo

      Class.new(c) do
        sig {returns(Integer)}
        def self.foo
          10
        end
      end
    end
  end

  it "allows redefining a regular instance method to be final" do
    Class.new do
      extend T::Sig
      def foo; end
      sig(:final) {void}
      def foo; end
    end
  end

  it "allows redefining a regular class method to be final" do
    Class.new do
      extend T::Sig
      def self.foo; end
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids overriding a final instance method" do
    c = Class.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        def foo; end
      end
    end
    assert_overridden_err('foo', CLASS_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "forbids overriding a final class method" do
    c = Class.new do
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        def self.foo; end
      end
    end
    assert_overridden_err('foo', CLASS_CLASS_REGEX_STR, CLASS_CLASS_REGEX_STR, __LINE__ - 7, __LINE__ - 3, err)
  end

  it "allows toggling a final method's visibility in the same class" do
    Class.new do
      extend T::Sig
      sig(:final) {void}
      private def foo; end

      sig(:final) {void}
      def bar; end
      private :bar
      public :bar
    end
  end

  it "allows declaring a final instance method and a final class method with the same name" do
    c = Class.new do
      extend T::Sig
      sig(:final) {returns(Symbol)}
      def foo
        :instance
      end

      sig(:final) {returns(Symbol)}
      def self.foo
        :class
      end
    end

    assert_equal(:instance, c.new.foo)
    assert_equal(:class, c.foo)
  end

  it "allows declaring a final class method and a non-final instance method with the same name" do
    c = Class.new do
      extend T::Sig
      sig(:final) {returns(Symbol)}
      def self.foo
        :class
      end

      sig {returns(Symbol)}
      def foo
        :instance
      end
    end

    assert_equal(:instance, c.new.foo)
    assert_equal(:class, c.foo)
  end

  it "allows declaring a final instance method and a non-final class method with the same name" do
    c = Class.new do
      extend T::Sig
      sig(:final) {returns(Symbol)}
      def foo
        :instance
      end

      sig {returns(Symbol)}
      def self.foo
        :class
      end
    end

    assert_equal(:instance, c.new.foo)
    assert_equal(:class, c.foo)
  end

  it "forbids toggling a final method's visibility in a child class" do
    c = Class.new do
      extend T::Sig

      sig(:final) {void}
      private def becomes_public; end

      sig(:final) {void}
      def becomes_private; end

      sig(:final) {void}
      protected def protected_becomes_private; end
    end
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        public :becomes_public
      end
    end
    assert_overridden_err('becomes_public', CLASS_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 13, __LINE__ - 3, err)
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        private :becomes_private
      end
    end
    assert_overridden_err('becomes_private', CLASS_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 16, __LINE__ - 3, err)
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        private :protected_becomes_private
      end
    end
    assert_overridden_err('protected_becomes_private', CLASS_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 19, __LINE__ - 3, err)
  end

  it "forbids overriding a final method from an included module" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m
        def foo; end
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 8, __LINE__ - 3, err)
  end

  it "forbids overriding a final method from an extended module" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        extend m
        def self.foo; end
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_CLASS_REGEX_STR, __LINE__ - 8, __LINE__ - 3, err)
  end

  it "forbids overriding a final method by including two modules" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m2, m1
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 10, __LINE__ - 3, err)
  end

  it "forbids overriding a final method by extending two modules" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        extend m2, m1
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 10, __LINE__ - 3, err)
  end

  it "allows calling final methods" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def self.n0; end
      sig(:final) {params(x: Integer).void}
      def self.n1(x); end
      sig(:final) {params(x: Integer, y: Integer).void}
      def self.n2(x, y); end
      sig(:final) {params(x: Integer, y: Integer, z: Integer).void}
      def self.n3(x, y, z); end
    end
    m.n0
    m.n1 1
    m.n2 1, 2
    m.n3 1, 2, 3
  end

  it "calls a user-defined included" do
    m = Module.new do
      @calls = 0
      extend T::Sig
      sig(:final) {returns(Integer)}
      def self.calls
        @calls
      end
      def self.included(x)
        @calls += 1
      end
    end
    Class.new do
      include m
    end
    assert_equal(1, m.calls)
    Class.new do
      include m
    end
    assert_equal(2, m.calls)
  end

  it "calls a user-defined extended" do
    m = Module.new do
      @calls = 0
      extend T::Sig
      sig(:final) {returns(Integer)}
      def self.calls
        @calls
      end
      def self.extended(x)
        @calls += 1
      end
    end
    Class.new do
      extend m
    end
    assert_equal(1, m.calls)
    Class.new do
      extend m
    end
    assert_equal(2, m.calls)
  end

  it "calls an exotic user-defined included" do
    m2 = Module.new do
      def self.included(arg)
        arg.include(Module.new do
          extend T::Sig
          sig(:final) {void}
          def foo; end
        end)
      end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m2
        def foo; end
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 10, __LINE__ - 3, err)
  end

  it "forbids overriding through many levels of include" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      include m1
    end
    m3 = Module.new do
      include m2
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m3
        def foo; end
      end
    end
    assert_overridden_err('foo', MODULE_REGEX_STR, CLASS_REGEX_STR, __LINE__ - 14, __LINE__ - 3, err)
  end

  it "allows including modules again" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    Module.new do
      include m1, m1
    end
  end

  it "allows extending modules again" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    Module.new do
      extend m1, m1
    end
  end

  it "has a good error if you use the wrong syntax" do
    err = assert_raises(ArgumentError) do
      m = Module.new do
        extend T::Sig
        sig {final.void}
        def self.foo; end
      end
      m.foo
    end
    assert_includes(err.message, "The syntax for declaring a method final is `sig(:final) {...}`, not `sig {final. ...}`")
  end
end
