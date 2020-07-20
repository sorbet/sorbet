# typed: true

Range.new(1, 10, true)
Range.new(1.0, 100.0, false)
Range.new('a', 'z')
Range.new(0.5, 2.5).step(0.5)
Range.new(0.5, 2.5).step(1/2r)
Range.new(0.5, 2.5).step(2+1i) # error: Expected `T.any(Integer, Float, Rational)` but found `Complex` for argument `n`

(1..10)
(1.0...20.0)
('a'..'z')
(:a...:z)

# bsearch
T.reveal_type((0.0..10.0).bsearch {|x| x >= 3}) # error: Revealed type: `T.nilable(Float)`
T.reveal_type((1..10).bsearch {|x| x == 1}) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type((1..10).bsearch {|x| x }) # error: Revealed type: `T.nilable(Integer)`
