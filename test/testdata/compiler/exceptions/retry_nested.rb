# frozen_string_literal: true
# typed: true
# compiled: true

should_raise = T.let(true, T::Boolean)

begin
  raise "foo" if should_raise
rescue
  begin
    puts "body"
  rescue
  else
    should_raise = false
    retry
  end
end
