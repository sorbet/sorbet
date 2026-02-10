# typed: __STDLIB_INTERNAL
class Sorbet::Private::Static::PackageSpec
  sig { params(arg0: T.class_of(Sorbet::Private::Static::PackageSpec), uses_internals: T::Boolean).void }
  def self.import(arg0, uses_internals: false); end

  sig { params(arg0: T.class_of(Sorbet::Private::Static::PackageSpec), only: T.nilable(String)).void }
  def self.test_import(arg0, only: nil); end

  sig { params(arg0: T.untyped).void }
  def self.export(arg0); end

  sig { params(arg0: T.untyped).void }
  def self.visible_to(arg0); end

  sig { void }
  def self.export_all!; end

  sig { params(arg0: String).void }
  def self.strict_dependencies(arg0); end

  sig { params(arg0: String).void }
  def self.layer(arg0); end

  sig { params(min_typed_level: String, tests_min_typed_level: String).void }
  def self.sorbet(min_typed_level:, tests_min_typed_level:); end

  sig { void }
  def self.prelude_package; end

  sig { void }
  def self.test!(); end
end
