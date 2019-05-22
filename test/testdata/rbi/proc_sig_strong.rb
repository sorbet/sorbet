# typed: strong

class A
  extend T::Sig

  sig {params(block: T.proc.void).void}
  def foo(&block); end
end
