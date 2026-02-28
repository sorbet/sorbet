# typed: true

# <=> returns Integer for comparable types, T.nilable(Integer) for incomparable types
T.reveal_type(Rational(1, 2) <=> 2) # error: type: `Integer`
T.reveal_type(Rational(1, 2) <=> 2.0) # error: type: `Integer`
T.reveal_type(Rational(1, 2) <=> Rational(1, 3)) # error: type: `Integer`
T.reveal_type(Rational(1, 2) <=> BigDecimal('1.0')) # error: type: `Integer`
T.reveal_type(Rational(1, 2) <=> 'incompatible') # error: type: `T.nilable(Integer)`
