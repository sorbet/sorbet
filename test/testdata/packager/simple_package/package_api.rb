# typed: strict

class PackageSpec
  extend T::Sig

  sig{params(x: T.untyped).void}
  def self.import(x)
  end

  sig{params(x: T.untyped).void}
  def self.export(x)
  end

  sig{params(x: T.untyped).void}
  def self.export_methods(*x)
  end
end
