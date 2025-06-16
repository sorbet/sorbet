# typed: true
Time.at(1)
Time.at(Time.now)
Time.at(0, 1589855340108, :milliseconds)

Time.now + 1
Time.now + 1.0
Time.now + 1.0r

Time.now + nil
#          ^^^ error: Expected `T.any(Integer, Float, Rational)` but found `NilClass` for argument `arg0`
Time.now + "foo"
#          ^^^^^ error: Expected `T.any(Integer, Float, Rational)` but found `String("foo")` for argument `arg0`

Time.now - 1
Time.now - 1.0
Time.now - 1.0r

t1 = Time.now
t2 = t1 - 10
t1 - t2

Time.now - nil
#          ^^^ error: Expected `Time` but found `NilClass` for argument `arg0`
Time.now - "foo"
#          ^^^^^ error: Expected `Time` but found `String("foo")` for argument `arg0`
