# typed: strict

# Depends on both `AliasPackage` (which defines the alias) and `TargetPackage` (which defines `Thing`)
AliasPackage::Alias::Thing.new

# Depends on `Outer::Inner`, but not on `Outer`, whose name is only a prefix here
Outer::Inner::InnerThing.new
