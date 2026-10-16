# typed: true

class Consumer::Value < Target::Value
  BAD = T.let("wrong", Integer)
  Extra::Value
end
