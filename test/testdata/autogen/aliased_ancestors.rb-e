# typed: true
# Make sure we can resolve references that go through an ancestor that has
# multiple levels of aliasing.

class Foo
  BAR = 1
end

Alias1 = Foo
Alias2 = Alias1

class Child < Alias2
  BAR
end
