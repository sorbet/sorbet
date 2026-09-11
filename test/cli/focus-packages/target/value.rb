# typed: true

class Target::Value < Lib::Base::Value
  BAD = T.let("wrong", Integer)

  # Prelude monkey patches remain available when upstream inference is skipped.
  T.assert_type!("target".prelude_value, Integer)
end
