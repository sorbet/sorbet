# typed: true

255[-1]
255[1..4]
255[1..]
255[..4]

# <=> returns Integer for comparable types, T.nilable(Integer) for incomparable types
T.reveal_type(1 <=> 2) # error: type: `Integer`
T.reveal_type(1 <=> 2.0) # error: type: `Integer`
T.reveal_type(1 <=> Rational(1, 2)) # error: type: `Integer`
T.reveal_type(1 <=> BigDecimal('1.0')) # error: type: `Integer`
T.reveal_type(1 <=> 'incompatible') # error: type: `T.nilable(Integer)`
