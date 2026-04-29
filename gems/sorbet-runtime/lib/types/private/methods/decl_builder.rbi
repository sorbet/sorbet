# typed: true

module T::Private::Methods
  class Declaration
    # TODO(jez) Can get the rest of these once ARG_NOT_PROVIDED is a module type

    sig { returns(Module) }
    attr_accessor :mod

    sig {returns(T::Hash[Symbol, T.untyped])}
    attr_accessor :params

    sig {returns(Object)}
    attr_accessor :returns

    sig {returns(BasicObject)}
    attr_accessor :bind

    sig {returns(String)}
    attr_accessor :mode

    sig {returns(T.nilable(Symbol))}
    attr_accessor :checked

    sig {returns(T::Boolean)}
    attr_accessor :finalized

    sig {returns(T.nilable(T::Array[T.untyped]))}
    attr_accessor :on_failure

    sig {returns(T.any(T::Boolean, Symbol))}
    attr_accessor :override_allow_incompatible

    sig {returns(T.any(T::Array[Symbol], T.class_of(ARG_NOT_PROVIDED)))}
    attr_accessor :type_parameters

    sig {returns(T::Boolean)}
    attr_accessor :raw

    sig do
      params(
        mod: Module,
        params: Object,
        returns: Object,
        bind: T.untyped,
        mode: String,
        checked: T.nilable(Symbol),
        finalized: T::Boolean,
        on_failure: T.any(T::Array[T.untyped], T.class_of(ARG_NOT_PROVIDED)),
        override_allow_incompatible: T.any(T::Boolean, Symbol),
        type_parameters: T.any(T::Array[Symbol], T.class_of(ARG_NOT_PROVIDED)),
        raw: T::Boolean,
      )
        .void
    end
    def initialize(
      mod,
      params,
      returns,
      bind,
      mode,
      checked,
      finalized,
      on_failure,
      override_allow_incompatible,
      type_parameters,
      raw
    ); end
  end

  # NOTE:
  # Most of the DeclBuilder methods are in rbi/sorbet/builder.rbi,
  # because this is nominally a "public" API, via sigs.
  #
  # The ones that are here are the "private" ones used only for the implementation
  class DeclBuilder
    sig {returns(Declaration)}
    def decl; end

    sig do
      params(
        mod: Module,
        raw: T::Boolean,
        abstract: T.nilable(T::Boolean),
        override: T.nilable(T.any(T::Boolean, Symbol)),
        overridable: T.nilable(T::Boolean)
      ).void
    end
    def initialize(mod, raw, abstract, override, overridable)
      @decl = T.let(nil, Declaration)
    end

    sig {returns(DeclBuilder)}
    def finalize!; end

    FROZEN_HASH = T.let({}, T::Hash[T.untyped, T.untyped])
    FROZEN_ARRAY = T.let([], T::Array[T.untyped])
  end
end
