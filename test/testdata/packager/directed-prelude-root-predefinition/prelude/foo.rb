# typed: true

# This invalid definition must be isolated from the real `MyPackage::Foo`, even though an explicit root scope in a
# prelude package is normally allowed. Otherwise this stratum creates a symbol owned by a later package, and the
# owner's type member crashes when it is entered. Whether this definition should also receive a diagnostic is a
# separate policy question.
module ::MyPackage
  class Foo
  end
end
