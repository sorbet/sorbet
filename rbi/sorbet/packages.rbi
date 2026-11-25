# typed: __STDLIB_INTERNAL
class Sorbet::Private::Static::PackageSpec
  sig { params(arg0: T.class_of(Sorbet::Private::Static::PackageSpec), uses_internals: T::Boolean).void }
  def self.import(arg0, uses_internals: false); end

  sig { params(arg0: T.untyped).void }
  def self.visible_to(arg0); end

  sig { params(arg0: String).void }
  def self.strict_dependencies(arg0); end

  sig { params(arg0: String).void }
  def self.layer(arg0); end

  # `tests_min_typed_level` no longer has any effect. It is accepted so that `__package.rb` files can delete it
  # independently of upgrading Sorbet.
  sig { params(min_typed_level: String, tests_min_typed_level: T.nilable(String)).void }
  def self.sorbet(min_typed_level:, tests_min_typed_level: nil); end

  sig { void }
  def self.prelude!; end

  sig { void }
  def self.test!(); end

  # Requires every package nested under this one to `import` it (or a
  # package in between). In exchange, the namespaces on the path to this
  # package's nested packages may define behavior, ancestors, and generics,
  # and may be classes.
  sig { void }
  def self.imported_by_nested_packages!; end
end
