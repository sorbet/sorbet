# typed: true

255[-1]
255[1..4]
255[1..]
255[..4]

# allbits? takes an Integer mask and return a Boolean
T.reveal_type(0b1010.allbits?(0b1000)) # error: type: `T::Boolean`
0b1010.allbits?(:nope) # error: Expected `Integer` but found `Symbol(:nope)` for argument `mask`

# anybits? takes an Integer mask and return a Boolean
T.reveal_type(0b1010.anybits?(0b1000)) # error: type: `T::Boolean`
0b1010.anybits?(:nope) # error: Expected `Integer` but found `Symbol(:nope)` for argument `mask`

# <=> returns Integer for comparable types, T.nilable(Integer) for incomparable types
T.reveal_type(1 <=> 2) # error: type: `Integer`
T.reveal_type(1 <=> 2.0) # error: type: `Integer`
T.reveal_type(1 <=> Rational(1, 2)) # error: type: `Integer`
T.reveal_type(1 <=> BigDecimal('1.0')) # error: type: `Integer`
T.reveal_type(1 <=> 'incompatible') # error: type: `T.nilable(Integer)`
