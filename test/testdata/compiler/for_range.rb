# frozen_string_literal: true
# typed: true
# compiled: true

def foo(a)
  for i in 0...a
    puts "hi"
  end
end

foo(10)
