# typed: true

module Sigish
  T::Sig::WithoutRuntime.sig {params(blk: T.proc.void).void}
  def self.sigish(&blk)
    yield
  end

  def self.params
    91
  end

  puts sigish {params}
end
