# typed: true
class Bar
  C = 1
end
Foo = Bar
Baz = Foo
Qux = Baz
Qux::C
