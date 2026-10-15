# typed: true

# T::DefMods is a public API and thus should have an RBI in `rbi/`
# (inside Sorbet's payload).
module T::DefMods
  sig { params(method_name: Symbol).returns(Symbol) }
  def abstract(method_name); end

  sig { params(method_name: Symbol, allow_incompatible: T.any(T::Boolean, Symbol)).returns(Symbol) }
  def override(method_name, allow_incompatible: false); end

  sig { params(method_name: Symbol).returns(Symbol) }
  def final(method_name); end

  sig { params(method_name: Symbol).returns(Symbol) }
  def overridable(method_name); end
end
