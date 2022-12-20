# typed: true

class A
  extend T::Sig

  # it doesn't make sense to use `.bind` for
  # anything other than a `&blk` argument
  sig {params(f: T.proc.bind(A).void).void}
  #           ^ error: Using `bind` is not permitted here
  def self.takes_fn(f)
  end
end
