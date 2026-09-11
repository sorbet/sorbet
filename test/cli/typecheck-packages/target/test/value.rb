# typed: true

class Test::Target::Value < TestSupport::Value
  BAD = T.let("wrong", Integer)
end
