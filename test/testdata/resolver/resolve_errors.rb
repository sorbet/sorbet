# typed: true
Foo # error: Unable to resolve constant `Foo`
Baz:: # error: MULTI
Bar # error: Unable to resolve constant `Bar`
module A; end
A::Quux # error: Unable to resolve constant `Quux`
