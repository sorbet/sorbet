# typed: true

class Define
  Data.define
  Data.define(:x, :y)
  Data.define("x", "y")

  Data.define(:x) do
    # Takes a block
  end
end

NotFound = Data.define
Point = Data.define(:x, :y)
SquaredPoint = Data.define(:x, :y) do
  def initialize(x:, y:)
    super(x: x * 2, y: y * 2)
  end
end

class ValidInitializers
  T.assert_type!(NotFound, T.class_of(NotFound))
  not_found = NotFound.new
  T.assert_type!(not_found, NotFound)

  T.assert_type!(Point, T.class_of(Point))

  point = Point.new(1, 2)
  T.reveal_type(point.x) # error: Revealed type: `T.untyped`
  T.reveal_type(point.y) # error: Revealed type: `T.untyped`
  T.assert_type!(point, Point)
  Point.new(1)
  Point.new(1, 2, 3)
  #               ^ error: Too many arguments provided for method `Point.new`. Expected: `0..2`, got: `3`

  point = Point.new(x: 1, y: 2)
  T.reveal_type(point.x) # error: Revealed type: `T.untyped`
  T.reveal_type(point.y) # error: Revealed type: `T.untyped`
  T.assert_type!(point, Point)
  Point.new(x: 1, y: 2, z: 3)
  Point.new(x: 1, z: 3)

  Point[1, 2]
  Point[1, 2, 3]
  Point[x: 1, y: 2]
  Point[x: 1, y: 2, z: 3]
  Point[x: 1, z: 3]

  T.assert_type!(SquaredPoint, T.class_of(SquaredPoint))

  squared_point = SquaredPoint.new(1, 2)
  T.reveal_type(squared_point.x) # error: Revealed type: `T.untyped`
  T.reveal_type(squared_point.y) # error: Revealed type: `T.untyped`
  T.assert_type!(squared_point, SquaredPoint)
  SquaredPoint.new(1)
  SquaredPoint.new(1, 2, 3)
  #                      ^ error: Too many arguments provided for method `SquaredPoint.new`. Expected: `0..2`, got: `3`

  squared_point = SquaredPoint.new(x: 1, y: 2)
  squared_point.x
  squared_point.y
  T.assert_type!(squared_point, SquaredPoint)
  SquaredPoint.new(x: 1, y: 2, z: 3)
  SquaredPoint.new(x: 1, z: 3)

  SquaredPoint[1, 2]
  SquaredPoint[1, 2, 3]
  SquaredPoint[x: 1, y: 2]
  SquaredPoint[x: 1, y: 2, z: 3]
  SquaredPoint[x: 1, z: 3]
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

module TypedData
  Money = Data.define(:amount, :currency) do
    extend T::Sig

    sig { params(amount: Numeric, currency: String).void }
    def initialize(amount:, currency:) = super
  end

  money = Money.new(amount: 10, currency: "CAD")
  T.assert_type!(money.amount, Numeric)
  T.assert_type!(money.currency, String)
  Money.new(amount: "10", currency: "CAD") # error: Expected `Numeric` but found `String("10")` for argument `amount`

  # Providing a sig for initialize means that `.new` will be typechecked and can
  # only support keyword arguments. Sorbet doesn't support overloads that rely
  # on the presence or absence of keyword arguments

  Money.new(1, "USD")
  #         ^^^^^^^^ error: Missing required keyword argument `amount` for method `TypedData::Money#initialize`
  #         ^^^^^^^^ error: Missing required keyword argument `currency` for method `TypedData::Money#initialize`
  #         ^^^^^^^^ error: Too many positional arguments provided for method `TypedData::Money#initialize`. Expected: `0`, got: `2`
  Money.new(1)
  #         ^ error: Missing required keyword argument `amount` for method `TypedData::Money#initialize`
  #         ^ error: Missing required keyword argument `currency` for method `TypedData::Money#initialize`
  #         ^ error: Too many positional arguments provided for method `TypedData::Money#initialize`. Expected: `0`, got: `1`
  Money.new(1, 2, 3)
  #         ^^^^^^^ error: Missing required keyword argument `amount` for method `TypedData::Money#initialize`
  #         ^^^^^^^ error: Missing required keyword argument `currency` for method `TypedData::Money#initialize`
  #         ^^^^^^^ error: Too many positional arguments provided for method `TypedData::Money#initialize`. Expected: `0`, got: `3`

  Money.new(amount: 1, currency: "USD")
  Money.new(amount: 1, currency: "USD", dirty: false)
  #                                     ^^^^^^^^^^^^ error: Unrecognized keyword argument `dirty` passed for method `TypedData::Money#initialize`
  Money.new(amount: 1, dirty: false)
  #         ^^^^^^^^^^^^^^^^^^^^^^^ error: Missing required keyword argument `currency` for method `TypedData::Money#initialize`
  #                    ^^^^^^^^^^^^ error: Unrecognized keyword argument `dirty` passed for method `TypedData::Money#initialize`

  # Square brackets remains unchecked
  Money[1, 2]
  Money[1, 2, 3]
  Money[x: 1, y: 2]
  Money[x: 1, y: 2, z: 3]
  Money[x: 1, z: 3]

  MoneyWithDefault = Data.define(:amount, :currency) do
    extend T::Sig

    sig { params(amount: Numeric, currency: String).void }
    def initialize(amount:, currency: "USD") = super
  end
  money_with_default = MoneyWithDefault.new(amount: 10, currency: "CAD")
  T.assert_type!(money_with_default.amount, Numeric)
  T.assert_type!(money_with_default.currency, String)
  MoneyWithDefault.new(amount: 10)
  MoneyWithDefault.new # error: Missing required keyword argument `amount` for method `TypedData::MoneyWithDefault#initialize`

  MoneyWithOtherMethods = Data.define(:amount, :currency) do
    extend T::Sig

    sig { params(amount: Numeric, currency: String).void }
    def initialize(amount:, currency:) = super

    sig { returns(String) }
    def display_string = "#{amount} #{currency}"
  end
  money_with_other_methods = MoneyWithOtherMethods.new(amount: 10, currency: "CAD")
  T.assert_type!(money_with_other_methods.amount, Numeric)
  T.assert_type!(money_with_other_methods.currency, String)
  T.assert_type!(money_with_other_methods.display_string, String)
end

# if the body of initialize is anything but a `super` call that forwards
# everything, then we can't build typed accessors based on the initialize sig
# Maybe someday we can actually look at what `super` gets called with?
module UntypedForNow
  MoneyWithTypeCoercion = Data.define(:amount, :currency) do
    extend T::Sig

    sig { params(amount: T.any(Numeric, String, Time), currency: String).void }
    def initialize(amount:, currency:)
      super(amount: amount.to_i, currency:)
    end
  end

  money_with_type_coercion = MoneyWithTypeCoercion.new(amount: 10, currency: "CAD")
  T.reveal_type(money_with_type_coercion.amount) # error: Revealed type: `T.untyped`
  T.reveal_type(money_with_type_coercion.currency) # error: Revealed type: `T.untyped`

  # Even if we can't build typed accessors, the sig for initialize constrains `.new
  money_with_type_coercion = MoneyWithTypeCoercion.new(amount: :symbol_is_bad, currency: "CAD")
  #                                                            ^^^^^^^^^^^^^^ error: Expected `T.any(Numeric, String, Time)` but found `Symbol(:symbol_is_bad)` for argument `amount`

  MoneyWithExtraSteps = Data.define(:amount, :currency) do
    extend T::Sig

    sig { params(amount: Numeric, currency: String).void }
    def initialize(amount:, currency:)
      puts("something")
      super
    end
  end


  money_with_extra_steps = MoneyWithExtraSteps.new(amount: 10, currency: "CAD")
  T.reveal_type(money_with_extra_steps.amount) # error: Revealed type: `T.untyped`
  T.reveal_type(money_with_extra_steps.currency) # error: Revealed type: `T.untyped`
end

module BadInitializers
  A = Data.define(:x, :y) do
    def initialize(x:) # error: Method `BadInitializers::A#initialize` redefined without matching argument count. Expected: `2`, got: `1`
      super
    end
  end
  AA = Data.define(:x, :y) do
    extend T::Sig

    sig { params(x: Numeric).void }
    def initialize(x:) # error: Method `BadInitializers::AA#initialize` redefined without matching argument count. Expected: `2`, got: `1`
      super
    end
  end

  B = Data.define(:x, :y) do
    def initialize(x:, z:) # error: Method `BadInitializers::B#initialize` redefined with mismatched keyword argument name. Expected: `y`, got: `z`
      super
    end
  end
  BB = Data.define(:x, :y) do
    extend T::Sig

    sig { params(x: Numeric, z: Numeric).void}
    def initialize(x:, z:) # error: Method `BadInitializers::BB#initialize` redefined with mismatched keyword argument name. Expected: `y`, got: `z`
      super
    end
  end

  C = Data.define(:x, :y) do
    def initialize(x, y) # error: Method `BadInitializers::C#initialize` redefined with argument `x` as a non-keyword argument
      super
    end
  end
  CC = Data.define(:x, :y) do
    extend T::Sig

    sig { params(x: Numeric, y: Numeric).void }
    def initialize(x, y) # error: Method `BadInitializers::CC#initialize` redefined with argument `x` as a non-keyword argument
      super
    end
  end

  D = Data.define(:x, :y) do
    def initialize(x:, y:, z:) # error: Method `BadInitializers::D#initialize` redefined without matching argument count. Expected: `2`, got: `3`
      super
    end
  end
  DD = Data.define(:x, :y) do
    extend T::Sig

    sig { params(x: Numeric, y: Numeric, z: Numeric).void }
    def initialize(x:, y:, z:) # error: Method `BadInitializers::DD#initialize` redefined without matching argument count. Expected: `2`, got: `3`
      super
    end
  end

  E = Data.define(:x, :y) do
    def initialize(xy:) # error: Method `BadInitializers::E#initialize` redefined without matching argument count. Expected: `2`, got: `1`
      super(x: xy, y: xy)
    end
  end
  EE = Data.define(:x, :y) do
    extend T::Sig

    sig { params(xy: Numeric).void }
    def initialize(xy:) # error: Method `BadInitializers::EE#initialize` redefined without matching argument count. Expected: `2`, got: `1`
      super(x: xy, y: xy)
    end
  end
end
