# typed: true

  1.0.to_s(10)
#          ^^ error: Too many arguments provided for method `Float#to_s`. Expected: `0`, got: `1`

# <=> returns Integer for comparable types, T.nilable(Integer) for incomparable types
T.reveal_type(1.0 <=> 2) # error: type: `Integer`
T.reveal_type(1.0 <=> 2.0) # error: type: `Integer`
T.reveal_type(1.0 <=> Rational(1, 2)) # error: type: `Integer`
T.reveal_type(1.0 <=> BigDecimal('1.0')) # error: type: `Integer`
T.reveal_type(1.0 <=> 'incompatible') # error: type: `T.nilable(Integer)`
