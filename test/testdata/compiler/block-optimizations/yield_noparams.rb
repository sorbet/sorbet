# frozen_string_literal: true
# typed: true
# compiled: true

def foo
  yield
end



foo do
  puts "heey"
end

