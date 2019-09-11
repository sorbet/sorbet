# typed: true
class Test
  extend T::Sig

  sig do
    params(failures: T::Array[T::Hash[String, String]])
    .returns(T.nilable(String))
  end
  def message(failures)
    failure = failures&.first
    T.assert_type!(failure && failure['message'], T.nilable(String))
  end
end
