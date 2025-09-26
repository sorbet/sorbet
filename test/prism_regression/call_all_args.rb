# typed: false

 RECEIVER1.m1(1, 2, *[3], e: 5, &FORWARDED_BLOCK)

 RECEIVER2.m2(1, 2, *[3], e: 5) { |a, b, *c, d:, e:, **f, &block| "inline block" }

 RECEIVER3.m3(1, 2, *[3], e: 5) do |a, b, *c, d:, e:, **f, &block|
  "do-end block"
 end
