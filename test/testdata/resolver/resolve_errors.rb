# typed: true
Foo # error: Unable to resolve constant `Foo`
Baz::Bar # error: Unable to resolve constant `Baz`
module A; end
A::Quux # error: Unable to resolve constant `Quux`
