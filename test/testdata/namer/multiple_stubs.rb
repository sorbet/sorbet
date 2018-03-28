# @typed
Foo::Bar.baz(1) # error: Unable to resolve constant
Foo::Bar.baz(2) # we do NOT report error here
