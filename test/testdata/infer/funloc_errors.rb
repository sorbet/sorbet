# typed: strict
extend T::Sig

sig {void}
def returns_void; end

returns_void.foo {puts 'hello'}
#            ^^^ error: Cannot call method `foo` on void type

T.proc.void.new {puts 'hello'}
#           ^^^ error: Call to method `new` on `T.proc.void` mistakes a type for a value

T.nilable(Integer).new {puts 'hello'}
#                  ^^^ error: Call to method `new` on `T.nilable(Integer)` mistakes a type for a value

T::Array[Integer].map {|x| puts x}
#                 ^^^ error: Call to method `map` on `T::Array[Integer]` mistakes a type for a value
