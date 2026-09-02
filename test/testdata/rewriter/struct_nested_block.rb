# typed: strict

class StructInBlock
  1.times do
    Generated = Struct.new(:value)
  end

  T.assert_type!(Generated.new(1), Generated)
  Generated.new(1).value
end

class StructInNestedBlock
  1.times do
    local_struct = Struct.new(:value)
    T.assert_type!(local_struct, Struct)

    1.times do
      Generated = Struct.new(:value)

      T.assert_type!(Generated.new(1), Generated)
      Generated.new(1).value
    end
  end
end
