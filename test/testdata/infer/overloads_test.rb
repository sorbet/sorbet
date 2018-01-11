# @typed
# Context::permitOverloadDefinitions is aware of this file name. Don't rename this file.
class HasOverloads
  standard_method(
    {},
    returns: Opus::Types.untyped()
  )
  def make_untyped
  end

  standard_method(
    {},
    returns: Integer
  )
  standard_method(
    {
      _: String,
    },
    returns: String
  )
  standard_method(
      {
        _: Exception,
      },
      returns: NilClass
  )
  standard_method(
    {
      _: Class,
      _1: String,
      _2: Opus::Types.array_of(String),
    },
    returns: Symbol
  )
  def overloaded(_=_, _1=_, _2=_);
    make_untyped
  end

  standard_method(
    {
      a: Class,  # error: Malformed standard_method
      b: String, # error: Malformed standard_method
    },
    returns: Integer
  )
  standard_method(
    {
      b: Class,  # error: Malformed standard_method
      a: String, # error: Malformed standard_method
    },
    returns: Symbol
  )
  def invalid_overloaded(a:, b:);
    make_untyped
  end
end

class Foo
  def test
    h = HasOverloads.new
    Opus::Types.assert_type!(h.overloaded(), Integer)
    Opus::Types.assert_type!(h.overloaded("s"), String)
    Opus::Types.assert_type!(h.overloaded(Exception.new), NilClass)
    Opus::Types.assert_type!(h.overloaded(self.class), Symbol)
    h.overloaded(1) # error: does not match expected type
                    # should ask for string
    h.overloaded("1", 2) # error: does not match expected type
  end
end
