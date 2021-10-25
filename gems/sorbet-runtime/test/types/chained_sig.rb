# frozen_string_literal: true

require_relative '../test_helper'

class Opus::Types::Test::ChainedSigTest < Critic::Unit::UnitTest
  describe "duplicate invocations" do
    it "raises when trying use abstract twice" do
      klass = Class.new do
        extend T::Sig

        sig.abstract {abstract.void}
        def bar; end
      end

      assert_raises(ArgumentError, ".abstract cannot be repeated in a single signature") do
        klass.new.bar
      end
    end

    it "raises when trying use final twice in a chained style" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, "You can't call .final multiple times in a signature.") do
        Class.new do
          extend T::Sig

          sig.final.final {void}
          def bar; end
        end
      end
    ensure
      cleanup_leftover_declaration
    end

    it "raises when trying use final twice" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, ".final cannot be repeated in a single signature") do
        Class.new do
          extend T::Sig

          sig(:final).final {void}
          def bar; end
        end
      end

    ensure
      cleanup_leftover_declaration
    end

    it "raises when trying use override twice" do
      interface = Module.new do
        extend T::Sig
        extend T::Helpers

        interface!

        sig.abstract {void}
        def baz; end
      end

      klass = Class.new do
        extend T::Sig
        include interface

        sig.override {override.void}
        def baz; end
      end

      assert_raises(ArgumentError, ".override cannot be repeated in a single signature") do
        klass.new.baz
      end
    end

    it "raises when trying use overridable twice" do
      klass = Class.new do
        extend T::Sig

        sig.overridable {overridable.void}
        def baz; end
      end

      assert_raises(ArgumentError, ".overridable cannot be repeated in a single signature") do
        klass.new.baz
      end
    end
  end

  describe "duplicate sig blocks" do
    it "raises when chaining overridable after a block has already been set" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, "Cannot define two separate signature blocks") do
        Class.new do
          extend T::Sig

          sig.final {void}.overridable {void}
          def baz; end
        end
      end
    ensure
      cleanup_leftover_declaration
    end

    it "raises when chaining override after a block has already been set" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, "Cannot define two separate signature blocks") do
        Class.new do
          extend T::Sig

          sig.final {void}.override {void}
          def baz; end
        end
      end
    ensure
      cleanup_leftover_declaration
    end

    it "raises when chaining abstract after a block has already been set" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, "Cannot define two separate signature blocks") do
        Class.new do
          extend T::Sig

          sig.final {void}.abstract {void}
          def baz; end
        end
      end
    ensure
      cleanup_leftover_declaration
    end

    it "raises when chaining final after a block has already been set" do
      assert_raises(T::Private::Methods::DeclBuilder::BuilderError, "Cannot define two separate signature blocks") do
        Class.new do
          extend T::Sig

          sig.override {void}.final {void}
          def baz; end
        end
      end
    ensure
      cleanup_leftover_declaration
    end
  end

  describe "without runtime sigs" do
    it "can also used the chained syntax for abstract" do
      Class.new do
        T::Sig::WithoutRuntime.sig.abstract {void}
        def foo; end
      end
    end

    it "can also used the chained syntax for final" do
      Class.new do
        T::Sig::WithoutRuntime.sig.final {void}
        def foo; end
      end
    end

    it "can also used the chained syntax for override" do
      interface = Module.new do
        extend T::Sig
        extend T::Helpers

        interface!

        T::Sig::WithoutRuntime.sig.abstract {void}
        def foo; end
      end

      Class.new do
        include interface

        T::Sig::WithoutRuntime.sig.override {void}
        def foo; end
      end
    end

    it "can also used the chained syntax for overridable" do
      Class.new do
        T::Sig::WithoutRuntime.sig.overridable {void}
        def foo; end
      end
    end
  end

  private

  def cleanup_leftover_declaration
    # Because some signatures raise before even running the block, we need to cleanup after ourselves or else the other
    # tests think there's an active declaration without a corresponding method definition
    T::Private::DeclState.current.reset!
  end
end
