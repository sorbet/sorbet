# @typed
def main
  T.assert_type!(Opus, Module)
  T.assert_type!(T::Array, Module)
  T.junk # error: Method junk does not exist on <class:T>
end
