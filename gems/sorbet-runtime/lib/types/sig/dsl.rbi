# typed: true

# T::Sig::DSL is a public API and thus should have an RBI in `rbi/`
# (inside Sorbet's payload).
#
# However, we need to intentionally redefine the method with a wider signature
# so that sorbet-runtime can do the runtime type validation.
module T::Sig::DSL
  sig { params(method_name: Kernel).returns(Symbol) }
  def abstract(method_name); end

  # Intentionally redefine the method with a different signature for sorbet-runtime,
  # so we can do the runtime type validation
  sig { params(method_name: Kernel, allow_incompatible: T::Boolean).returns(Symbol) }
  def override(method_name, allow_incompatible: false); end

  # Intentionally redefine the method with a different signature for sorbet-runtime,
  # so we can do the runtime type validation
  sig { params(method_name: Kernel).returns(Symbol) }
  def final(method_name); end

  # Intentionally redefine the method with a different signature for sorbet-runtime,
  # so we can do the runtime type validation
  sig { params(method_name: Kernel).returns(Symbol) }
  def overridable(method_name); end
end
