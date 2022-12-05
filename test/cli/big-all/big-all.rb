# typed: true

module IFoo; end
class Foo
  include IFoo
end

T.let(Foo.new, T.all(Class, IFoo))
