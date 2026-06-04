# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::VisibilityTest < Critic::Unit::UnitTest
  it "allows public/public overrides" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "allows private/private overrides" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      private def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      private def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "allows a more permissive override" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      private def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "handles protected overrides" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      protected def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "errors if the override is less permissive than the base" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      private def foo; 0; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(child)
    end
    assert_includes(err.message, "Incompatible visibility")
    assert_includes(err.message, "at least as permissive")
  end

  it "knows that public < protected" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      protected def foo; 0; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(child)
    end
    assert_includes(err.message, "Incompatible visibility")
    assert_includes(err.message, "at least as permissive")
  end

  it "knows that protected < private" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      protected def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override.returns(Integer) }
      private def foo; 0; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(child)
    end
    assert_includes(err.message, "Incompatible visibility")
    assert_includes(err.message, "at least as permissive")
  end

  it "respects allow_override: :visibility" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override(allow_incompatible: :visibility).returns(Integer) }
      private def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "respects allow_override: true" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { abstract.returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { override(allow_incompatible: true).returns(Integer) }
      private def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "doesn't check visibility overrides unless `override` is explicit" do
    parent = Class.new do
      extend T::Sig, T::Helpers
      abstract!
      sig { returns(Integer) }
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { returns(Integer) }
      private def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end

  it "doesn't check visibility overrides if the parent is untyped" do
    parent = Class.new do
      def foo; end
    end
    child = Class.new(parent) do
      extend T::Sig, T::Helpers
      sig { returns(Integer) }
      private def foo; 0; end
    end

    T::Private::Abstract::Validate.validate_subclass(child)
  end
end
