# typed: true

BigDecimal.new('123')
BigDecimal.new(100)
BigDecimal.new(0.001)
BigDecimal.new(100, 2)
BigDecimal('123')
BigDecimal(999)
BigDecimal('123', 4)
BigDecimal::SIGN_NaN

BigDecimal('0.5').round
BigDecimal('0.5').round(1)
BigDecimal('0.5').round(1, BigDecimal::ROUND_HALF_EVEN)
BigDecimal('0.5').round(1, :up)
BigDecimal('0.5').round(1, 'up') # error: Expected `T.any(Integer, Symbol)` but found `String("up")` for argument `mode`

x = T.let(3, T.any(Integer, Float, Rational, BigDecimal))
BigDecimal(2) * x
BigDecimal(2) ** x
BigDecimal(2) + x
BigDecimal(2) - x
BigDecimal(2) / x
BigDecimal(2) < x
BigDecimal(2) <= x
BigDecimal(2) <=> x
BigDecimal(2) > x
BigDecimal(2) >= x
BigDecimal(2).div(x)
BigDecimal(2).fdiv(x)
BigDecimal(2).modulo(x)
BigDecimal(2).power(x)
BigDecimal(2).quo(x)

# <=> returns Integer for comparable types, T.nilable(Integer) for incomparable types
T.reveal_type(BigDecimal('1.0') <=> 2) # error: type: `Integer`
T.reveal_type(BigDecimal('1.0') <=> 2.0) # error: type: `Integer`
T.reveal_type(BigDecimal('1.0') <=> Rational(1, 2)) # error: type: `Integer`
T.reveal_type(BigDecimal('1.0') <=> BigDecimal('2.0')) # error: type: `Integer`
T.reveal_type(BigDecimal('1.0') <=> 'incompatible') # error: type: `T.nilable(Integer)`
