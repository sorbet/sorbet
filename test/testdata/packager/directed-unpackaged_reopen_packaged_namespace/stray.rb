# error: stray.rb` does not belong to a package
# Omit the directory so the assertion also matches LSP paths.
# typed: true

# This file belongs to no package, which is already an error (3705, on line 1). `checkScopePackage` then also
# reports 5086 for every scope in the file whose symbol belongs to a package -- including `Opus::Foo::Extra`,
# which this file *defines* (it inherits `package = Opus::Foo` from its owner), so that message is misleading as
# well as redundant. Record the current behavior before making `checkScopePackage` return early when the file
# has no package, as `shouldCheckPackage` already does.
module Opus::Foo # error: Unpackaged code may not open `Opus::Foo`
  class Extra # error: Unpackaged code may not open `Opus::Foo::Extra`
    X = Integer
  end
end

# Qualified assignments and intermediate package prefixes must be isolated too.
Opus::Foo::Thing::FROM_STRAY = Integer
module Opus
  FROM_STRAY = Integer
end

# Entirely new root constants and payload reopenings must not leak either.
ROOT_VALUE = Integer
class RootClass; end
class String
  FROM_STRAY = Integer
end
