# typed: strict
# enable-packager: true
# enable-package-directed: true
# packager-layers: a

class ::Sorbet::Private::Static::PackageSpec
  sig {params(x: String).void}
  def self.custom_method(x); end
end
