# typed: false

module T; class Bar; end; class Foo; end; end

def foo
  tr1 = T::Foo.new
end

def bar
  T::Bar.new
end
