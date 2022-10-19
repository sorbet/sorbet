# typed: true
# enable-suggest-unsafe: true

extend T::Sig

class MyClass; end

sig {params(response: T::Hash[String, T::Hash[String, T::Array[T.noreturn]]]).void}
def example1(response)
  x = response.dig("", "", 0, 0)
  #                           ^ error: Unused arguments to `dig`
  T.reveal_type(x) # error: `NilClass`
end

sig {params(opts: T::Hash[String, T.any(Integer, T::Hash[String, String])]).void}
def example2(opts)
  x = opts.dig("data", "_id")
  #                    ^^^^^ error: Method `dig` does not exist on `Integer` component of `T.any(T::Hash[String, String], Integer)`
  T.reveal_type(x) # error: `T.untyped`
end

class MyClass
  def my_method; end
end

sig {params(opts: T::Array[MyClass]).void}
def example3(opts)
  x = opts.dig(0)
  T.reveal_type(x) # error: `T.nilable(MyClass)`
  x.my_method
  # ^^^^^ error: Method `my_method` does not exist on `NilClass` component of `T.nilable(MyClass)`
end

# TODO(jez) Some shape / tuple tests, to capture that we haven't implemented something special yet.
# TODO(jez) Some tests for keyword args
# TODO(jez) Test for `Struct` or other classes that have special `dig` implementation
