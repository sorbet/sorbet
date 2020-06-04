# frozen_string_literal: true
# typed: true
# compiled: true

should_raise = T.let(true, T::Boolean)

begin
  puts "body"
  raise "foo" if should_raise
rescue
  puts "rescue"
  should_raise = false
  retry
else
  puts "else"
ensure
  puts "ensure"
end
