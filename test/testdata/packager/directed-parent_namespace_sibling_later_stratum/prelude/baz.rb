# typed: true

# A root-level constant with the same name as a sibling subpackage of `Opus::Foo`. Ruby's lexical lookup
# never reaches this from inside `module Opus::Foo`, because it finds `Opus::Foo::Baz` first.
module ::Baz
end
