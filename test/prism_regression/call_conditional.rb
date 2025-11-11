# typed: false

self&.foo

self&.foo(1)

self&.foo(a: 1)

self&.foo(&forwarded_block)

self&.foo { |x| x }

self&.foo(*args)
