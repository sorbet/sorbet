# frozen_string_literal: true
# typed: true
# compiled: true

# This tests what happens when the variable introduced to hold the exception
# value doesn't end up in the closure for the function. The body of the
# begin/ensure clause on line 11 unconditionally returns, which causes the
# conditional branch at the end of that block based on the exception value to
# disappear.

def test
  begin
    return true
  ensure
    puts "ensure"
  end
end

puts test
