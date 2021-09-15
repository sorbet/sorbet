# compiled: true
# typed: true
# frozen_string_literal: true

foo = proc do |*args|
  puts args
end

foo.call(1,2,3,'hi')
