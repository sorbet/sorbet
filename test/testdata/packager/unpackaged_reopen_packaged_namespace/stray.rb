# error: File `
# ^ We can't assert the full pathname; see https://github.com/sorbet/sorbet/pull/3310
# typed: true

# This file belongs to no package, which is already an error (3705, on line 1). `checkScopePackage` then also
# reports 5086 for every scope in the file whose symbol belongs to a package -- including `Opus::Foo::Extra`,
# which this file *defines* (it inherits `package = Opus::Foo` from its owner), so that message is misleading as
# well as redundant. This snapshot records the current behavior; the follow-up is to return early from
# `checkScopePackage` when the file has no package, as `shouldCheckPackage` already does.
module Opus::Foo # error: Unpackaged code may not open `Opus::Foo`
  class Extra # error: Unpackaged code may not open `Opus::Foo::Extra`
    X = Integer
  end
end
