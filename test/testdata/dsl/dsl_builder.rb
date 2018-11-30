# typed: true
class TestDSLBuilder
  dsl_optional :opt_string, String
  dsl_optional :opt_int_defaulted, Integer, default: 0
  dsl_required :req_string, String
  dsl_required :implied_string, String, implied: "foo"
  dsl_optional :no_getter, String, skip_getter: true

  dsl_optional :root_const, ::Integer
end

class TestChild < TestDSLBuilder
  opt_string "hi"
  get_opt_string
  opt_string # error: Not enough arguments provided
  opt_string :nope # error: `Symbol(:"nope")` doesn't match `String` for argument `opt_string`
  opt_string nil # error: `NilClass` doesn't match `String` for argument `opt_string`
  opt_int_defaulted 17
  implied_string
  no_getter "I'm setting the value"
  get_no_getter # error: Method `get_no_getter` does not exist on `T.class_of(TestChild)`

  T.assert_type!(get_opt_string, T.nilable(String))
  T.assert_type!(get_opt_string, String) # error: does not have asserted type
  T.assert_type!(get_opt_int_defaulted, Integer)
  T.assert_type!(get_req_string, String)

  def test_instance_methods
    T.assert_type!(opt_string, T.nilable(String))
    T.assert_type!(opt_int_defaulted, Integer)
    T.assert_type!(req_string, String)
    no_getter # error: Method `no_getter` does not exist on `TestChild`
  end
end
