# @typed
class TestConfigatron
  def configatron_test
      T.assert_type!(configatron.test_bool, T.any(TrueClass, FalseClass))
      T.assert_type!(configatron.test_int, Integer)
      T.assert_type!(configatron.test_float, Float)
      T.assert_type!(configatron.test_string, String)
      T.assert_type!(configatron.test_nested.test_int, Integer)

      T.assert_type!(configatron, Configatron::RootStore)
      T.assert_type!(configatron.test_nested, Configatron::Store)
  end
end
