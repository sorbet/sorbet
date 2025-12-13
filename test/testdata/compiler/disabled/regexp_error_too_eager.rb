# frozen_string_literal: true
# typed: true
# compiled: true

puts "hi"
if T.unsafe(false)
  Regexp.new("?")
end
