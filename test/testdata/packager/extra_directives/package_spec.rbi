# typed: strict

class ::Sorbet::Private::Static::PackageSpec
  sig(:final) {params(x: String).void}
  def self.extra(x); end
end
