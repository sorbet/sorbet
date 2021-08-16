# frozen_string_literal: true
# typed: true
# compiled: true

def foo(a, b = 2, k:, g:6, **args)
  puts a, b, k, g, args.inspect
end
foo(1, k: 3, y: 7)
