# typed: true

 receiver.foo(1, 2, 3, d: 4, e: 5, f: 6, &forwarded_block)
#^^^^^^^^ error: Method `receiver` does not exist on `T.class_of(<root>)`
#                                         ^^^^^^^^^^^^^^^ error: Method `forwarded_block` does not exist on `T.class_of(<root>)`

 receiver.foo(1, 2, 3, d: 4, e: 5, f: 6) { |a, b, c, d:, e:, f:, &block| "inline block" }
#^^^^^^^^ error: Method `receiver` does not exist on `T.class_of(<root>)`

 receiver.foo(1, 2, 3, d: 4, e: 5, f: 6) do |a, b, c, d:, e:, f:, &block|
#^^^^^^^^ error: Method `receiver` does not exist on `T.class_of(<root>)`
  "do-end block"
 end
