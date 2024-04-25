# frozen_string_literal: true
# typed: true
# compiled: true

# Tests that we can catch custom exception types.

class E < StandardError; end

begin
  raise E, "custom error"
rescue E => e
  puts e.message
end

begin
  raise E, "custom error, caught as StandardError"
rescue => e
  puts e.message
end


class Ex < Exception; end

begin
  begin
    raise Ex, "custom exception"
  rescue => e
    puts e.message
  end
rescue Exception => e
  puts "Explicitly handled exception: #{e.message}"
end

