# typed: strict
class Example
  1.times do
    Generated = Struct.new(:value) # error: Constants must have type annotations
  end
end
