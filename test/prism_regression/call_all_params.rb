# typed: false

 receiver.foo(1, 2, *[3], d: 4, e: 5, f: 6, &forwarded_block)

 receiver.foo(1, 2, *[3], d: 4, e: 5, f: 6) { |a, b, c, d:, e:, f:, &block| "inline block" }

 receiver.foo(1, 2, *[3], d: 4, e: 5, f: 6) do |a, b, c, d:, e:, f:, &block|
  "do-end block"
 end
