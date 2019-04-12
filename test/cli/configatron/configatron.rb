# typed: true
class TestConfigatron
  def configatron_test
      T.assert_type!(configatron.test_bool, T::Boolean)
      T.assert_type!(configatron.test_int, Integer)
      T.assert_type!(configatron.test_float, Float)
      T.assert_type!(configatron.test_string, String)
      T.assert_type!(configatron.test_nested.test_int, Integer)

      T.assert_type!(configatron, Configatron::RootStore)
      T.assert_type!(configatron.test_nested, Configatron::Store)

      T.assert_type!(configatron.test_nested.key?(:junk), T::Boolean)
      T.assert_type!(configatron.key?(:junk), T::Boolean)
  end

  def lub_test(param)
    config = if param
      configatron.test_hash_a
    else
      configatron.test_hash_b
    end

    if config && 4
      "foo"
    end
    "bar"
  end
end
