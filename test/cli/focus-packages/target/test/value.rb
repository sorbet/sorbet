# typed: true

class Target::Test::Value < TestSupport::Value
  BAD = T.let("wrong", Integer)
end
