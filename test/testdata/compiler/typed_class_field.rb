# frozen_string_literal: true
# typed: true
# compiled: true

class Parent
  @@s = T.let(500, Integer)

  puts @@s
end

class Child < Parent
  puts @@s
end
