# typed: core
class Sorbet
  # Identical to `T::Sig`'s `sig` in semantics, but couldn't work at
  # runtime since it doesn't know `self`. Used in `rbi`s that don't `extend
  # T::Sig`.
  sig {params(blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
  def self.sig(&blk)
  end
end

