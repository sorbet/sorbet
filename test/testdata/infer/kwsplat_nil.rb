# typed: strict

class Example
  extend T::Sig
  extend T::Generic

  MaybeHash = type_member { {upper: T.nilable(T::Hash[T.untyped, T.untyped])} }

  sig { params(kwargs: T.untyped).void }
  def takes_kwargs(**kwargs)
  end

  sig { params(hash: T.nilable(T::Hash[T.untyped, T.untyped]), maybe_hash_tm: MaybeHash).void }
  def example(hash, maybe_hash_tm)
    { success: true, **hash }

    takes_kwargs(**nil)

    no_kwargs = nil
    takes_kwargs(**no_kwargs)

    takes_kwargs(**hash)

    takes_kwargs(**maybe_hash_tm) # error: Method `to_hash` does not exist on `NilClass` component
  end
end
