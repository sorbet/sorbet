# typed: true

xs = {
  foo: T.let(nil, T.nilable(Integer)),
  bar: nil,
}

T.reveal_type(xs) # error: Revealed type: `{foo: T.nilable(Integer), bar: NilClass} (shape of T::Hash[Symbol, T.nilable(Integer)])`

xs[:foo] = 1
xs[:bar] = 1 # error: Expected `NilClass` but found `Integer(1)` for key `Symbol(:bar)`
xs[:qux] # error: Key `Symbol(:qux)` not present in shape `{foo: T.nilable(Integer), bar: NilClass}`

options = {
  enable_foo: T.let(false, T::Boolean),
  enable_bar: T.let(false, T::Boolean),
  enable_qux: false,
}

T.reveal_type(options) # error: Revealed type: `{enable_foo: T::Boolean, enable_bar: T::Boolean, enable_qux: FalseClass} (shape of T::Hash[Symbol, T::Boolean])`

ARGV.each do |arg|
  case arg
  when 'enable_foo' then options[:enable_foo] = true
  when 'enable_bar' then options[:enable_bar] = true
  when 'enable_qux' then options[:enable_qux] = true # error: Expected `FalseClass` but found `TrueClass` for key `Symbol(:enable_qux)`
  end
end

T.reveal_type(options) # error: Revealed type: `{enable_foo: T::Boolean, enable_bar: T::Boolean, enable_qux: FalseClass} (shape of T::Hash[Symbol, T::Boolean])`
