# frozen_string_literal: true
# typed: true

class A < T::Struct
  prop :foo, T.nilable(String), enum: ['foo', 'bar']
end

a1 = A.new(foo: 'bar')
p a1.foo
a1.foo = 'foo'
p a1.foo
begin
  a1.foo = 'nope'
  p a1.foo
rescue
  p :ok
end
