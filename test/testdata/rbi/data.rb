# typed: true

class Define
  Data.define # error: Not enough arguments provided for method `Data.define`. Expected: `1+`, got: `0`
  Data.define(:x, :y)
  Data.define("x", "y")

  Data.define(:x) do
    # Takes a block
  end
end

Point = Data.define(:x, :y)

class ValidInitializers
  T.assert_type!(Point, T.class_of(Point))

  point = Point.new(1, 2)
  point.x
  point.y
  T.assert_type!(point, Point)

  point = Point.new(x: 1, y: 2)
  point.x
  point.y
  T.assert_type!(point, Point)

  point = Point[1, 2]
  point = Point[x: 1, y: 2]
end

class InstanceMethods
  point = Point.new(1, 2)
  other_point = Point.new(3, 4)

  # ==
  T.assert_type!(point == other_point, T::Boolean)
  T.assert_type!(point == 'random', T::Boolean)

  # eql?
  T.assert_type!(point.eql?(other_point), T::Boolean)
  T.assert_type!(point.eql?('random'), T::Boolean)

  # deconstruct
  T.assert_type!(point.deconstruct, T::Array[T.untyped])

  # deconstruct_keys
  T.assert_type!(
    point.deconstruct_keys(nil),
    T::Hash[Symbol, T.untyped]
  )
  T.assert_type!(
    point.deconstruct_keys([:x]),
    T::Hash[Symbol, T.untyped]
  )
  T.assert_type!(
    point.deconstruct_keys(["x", "y"]),
    T::Hash[Symbol, T.untyped]
  )

  # hash
  T.assert_type!(point.hash, Integer)

  # inspect
  T.assert_type!(point.inspect, String)

  # members
  T.assert_type!(point.members, T::Array[Symbol])

  # to_h
  T.assert_type!(point.to_h, T::Hash[T.untyped, T.untyped])
  point.to_h do |name, value|
    T.assert_type!(name, Symbol)
  end

  # with
  new_point = point.with(x: 5)
  T.assert_type!(new_point, Point)
end
