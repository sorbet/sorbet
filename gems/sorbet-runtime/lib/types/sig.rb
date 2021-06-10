# frozen_string_literal: true
# typed: strict

# Used as a mixin to any class so that you can call `sig`.
# Docs at https://sorbet.org/docs/sigs
module T::Sig
  module WithoutRuntime
    # At runtime, does nothing, but statically it is treated exactly the same
    # as T::Sig#sig. Only use it in cases where you can't use T::Sig#sig.
    def self.sig(arg0=nil, &blk); end

    original_verbose = $VERBOSE
    $VERBOSE = false

    # At runtime, only returns a fake declaration builder to allow chaining methods, but statically it is treated
    # exactly the same as T::Sig#sig. Only use it in cases where you can't use T::Sig#sig.
    T::Sig::WithoutRuntime.sig {params(arg0: T.nilable(Symbol), blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
    def self.sig(arg0=nil, &blk) # rubocop:disable Lint/DuplicateMethods
      DeclBuilder.new
    end

    # This fake version of DeclBuilder exists so that signatures without runtime don't have to pay the cost associated
    # with using the actual DeclBuilder
    class DeclBuilder
      T::Sig::WithoutRuntime.sig do
        params(
          _blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
        ).returns(DeclBuilder)
      end
      def abstract(&_blk)
        self
      end

      T::Sig::WithoutRuntime.sig do
        params(
          _blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
        ).returns(DeclBuilder)
      end
      def final(&_blk)
        self
      end

      T::Sig::WithoutRuntime.sig do
        params(
          _allow_incompatible: T::Boolean,
          _blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
        ).returns(DeclBuilder)
      end
      def override(_allow_incompatible: false, &_blk)
        self
      end

      T::Sig::WithoutRuntime.sig do
        params(
          _blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
        ).returns(DeclBuilder)
      end
      def overridable(&_blk)
        self
      end
    end

    $VERBOSE = original_verbose
  end

  # Declares a method with type signatures and/or
  # abstract/override/... helpers. See the documentation URL on
  # {T::Helpers}
  T::Sig::WithoutRuntime.sig {params(arg0: T.nilable(Symbol), blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
  def sig(arg0=nil, &blk)
    T::Private::Methods.declare_sig(self, Kernel.caller_locations(1, 1)&.first, arg0, &blk)
  end
end
