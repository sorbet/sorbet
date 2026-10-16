# typed: true

class CycleA::Value < Target::Value
  BAD = T.let("wrong", Integer)
end
