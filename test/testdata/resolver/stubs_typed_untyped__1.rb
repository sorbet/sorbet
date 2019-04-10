# typed: false
# This file is deliberately not typed. We are demonstrating a bug
# where we would process Foo::Bar here, suppress the error, and enter
# a stub symbol into the symbol table, meaning that we would not then
# trigger the error in the `typed: strict` _2 file.

module Foo
end
Bar # error: Unable to resolve constant `Bar`

Foo::Bar # error: Unable to resolve constant `Bar`
