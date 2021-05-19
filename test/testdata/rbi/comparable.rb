# typed: true

12.clamp(0, 100)
523.clamp(0, 100)
-3.123.clamp(0, 100)
'd'.clamp('a', 'f')
'z'.clamp('a', 'f')
12.clamp(0..100)
523.clamp(0..100)
-3.123.clamp(0..100)
'd'.clamp('a'..'f')
'z'.clamp('a'..'f')
-20.clamp(0..)
523.clamp(..100)
