# typed: strict
class TestDSLBuilder
  dsl_optional :opt_string, String
  dsl_optional :opt_int_defaulted, Integer, default: 0
  dsl_required :req_string, String

  dsl_optional :root_const, ::Integer
end

class TestChild < TestDSLBuilder
  opt_string "hi"
  opt_string :nope # error: does not match expected type
  opt_int_defaulted 17

  T.assert_type!(get_opt_string, T.nilable(String))
  T.assert_type!(get_opt_int_defaulted, Integer)
  T.assert_type!(get_req_string, String)

  def test_instance_methods
    T.assert_type!(opt_string, T.nilable(String))
    T.assert_type!(opt_int_defaulted, Integer)
    T.assert_type!(req_string, String)
  end
end
