# @typed
class Test
  sig(failures: T::Array[T::Hash[String, String]])
  .returns(T.nilable(String))
  def message(failures)
    failure = failures&.first
    T.assert_type!(failure && failure['message'], T.nilable(String))
  end
end
