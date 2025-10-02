# typed: false

 RECEIVER1.m1(1, 2, *[3, 4], 5, e: 6, **f, &FORWARDED_BLOCK)

 RECEIVER2.m2(1, 2, *[3, 4], 5, e: 6, **f) { |a, b, *c, d, e:, **f, &block| "inline block" }

 RECEIVER3.m3(1, 2, *[3, 4], 5, e: 6, **f) do |a, b, *c, d, e:, **f, &block|
  "do-end block"
 end
