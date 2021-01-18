# typed: strict

xs = {
  foo: nil,
  bar: false,
  qux: true,
}

xs[:foo] = 1
xs[:bar] = true
xs[:qux] = false

ys = {
  first: [],
  second: {},
}
ys[:first] = T::Array[Integer].new
ys[:second] = T::Hash[Symbol, String].new

zs = {
  'foo' => false,
  :bar => false,
}
zs['foo'] = true
zs[:bar] = true

spacing_is_important = {
  foo:nil,
}
spacing_is_important[:foo] = 1

if T.unsafe(nil)
  initialized_twice = {
    foo: nil,
  }
else
  initialized_twice = {
    foo: false,
  }
end
initialized_twice[:foo] = true
