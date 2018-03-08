# This file is deliberately not @typed. We are demonstrating a bug
# where we would process Foo::Bar here, suppress the error, and enter
# a stub symbol into the symbol table, meaning that we would not then
# trigger the error in the @typed _2 file.

module Foo
end
Bar
Foo::Bar
