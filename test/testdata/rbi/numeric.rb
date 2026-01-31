# typed: true

class MyNumeric < Numeric
end

n = MyNumeric.new
# <=> returns Integer for comparable types, NilClass for incomparable types
T.reveal_type(n <=> 2) # error: type: `Integer`
T.reveal_type(n <=> 2.0) # error: type: `Integer`
T.reveal_type(n <=> Rational(1, 2)) # error: type: `Integer`
T.reveal_type(n <=> BigDecimal('1.0')) # error: type: `Integer`
T.reveal_type(n <=> 'incompatible') # error: type: `NilClass`
