# frozen_string_literal: true
# typed: true

# Optional mixin providing `abstract`, `override`, `overridable`, and `final`
# as method-level DSL keywords. Use with `extend T::DefMods`.
#
# These are alternatives to writing modifiers inside a `sig { ... }` block:
#
#   sig { void }
#   abstract def foo; end
#
# is equivalent to:
#
#   sig { abstract.void }
#   def foo; end
#
# They all return the method name, so that they can be chained with methods
# like `private`. However, unlike those methods, these methods use the `sig`
# declaration to discover the most-recently-defined method, instead of needing
# `*_class_method` variants, like `private_class_method`.
module T::DefMods
  # ===== NOTE: Must keep in sync with `T::Sig`! ==============================
  include T::Private::Methods::MethodHooks
  include T::Private::Methods::SingletonMethodHooks

  private_class_method def self.included(other)
    return unless Module == other
    other.prepend(T::Private::Methods::MethodHooks)
  end

  private_class_method def self.extended(other)
    if other == T::Private::Methods::TOP_SELF
      Object.extend(T::Private::Methods::MethodHooks)
      return
    end

    return unless Module.===(other) && other.singleton_class?
    other.include(T::Private::Methods::SingletonMethodHooks)
  end
  # ===========================================================================

  def abstract(method_name)
    Kernel.raise TypeError.new("abstract accepts a Symbol, got #{method_name.class}") unless method_name.is_a?(Symbol)

    begin
      T::Private::Methods.declare_abstract(T.unsafe(self), method_name)
    rescue T::Private::Methods::DeclBuilder::BuilderError => e
      T::Configuration.sig_builder_error_handler(e, Kernel.caller_locations(1, 1)&.first)
    end

    method_name
  end

  def override(method_name, allow_incompatible: false)
    Kernel.raise TypeError.new("override accepts a Symbol, got #{method_name.class}") unless method_name.is_a?(Symbol)

    begin
      T::Private::Methods.declare_override(T.unsafe(self), method_name, allow_incompatible: allow_incompatible)
    rescue T::Private::Methods::DeclBuilder::BuilderError => e
      T::Configuration.sig_builder_error_handler(e, Kernel.caller_locations(1, 1)&.first)
    end

    method_name
  end

  def final(method_name)
    Kernel.raise TypeError.new("final accepts a Symbol, got #{method_name.class}") unless method_name.is_a?(Symbol)

    begin
      T::Private::Methods.declare_final(T.unsafe(self), method_name)
    rescue T::Private::Methods::DeclBuilder::BuilderError => e
      T::Configuration.sig_builder_error_handler(e, Kernel.caller_locations(1, 1)&.first)
    end

    method_name
  end

  def overridable(method_name)
    Kernel.raise TypeError.new("overridable accepts a Symbol, got #{method_name.class}") unless method_name.is_a?(Symbol)

    begin
      T::Private::Methods.declare_overridable(T.unsafe(self), method_name)
    rescue T::Private::Methods::DeclBuilder::BuilderError => e
      T::Configuration.sig_builder_error_handler(e, Kernel.caller_locations(1, 1)&.first)
    end

    method_name
  end
end
