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
