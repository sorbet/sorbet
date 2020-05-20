# frozen_string_literal: true
# typed: true
# compiled: true

# Tests forwarding the exception via a use of `raise` with no arguments

# `raise` with no arguments should forward the exception
puts "=== `raise` from rescue"
begin
  begin
    raise "foo"
  rescue => e
    puts e.message
    puts "caught"
    raise
  end
rescue => e
  puts "Forwarded: #{e.message}"
end

puts "=== `raise` from ensure"
begin
  begin
    raise "bar"
  rescue
  ensure
    raise
  end
rescue => e
  # This message should be empty if everything works correctly, as a use of
  # `raise` with no arguments outside of a `rescue` block is an error
  puts "Bad rethrow message: #{e.message}"
end

puts "=== `raise` from else ==="
begin
  begin
    raise "baz"
  rescue
    begin
    rescue
    else
      raise
    end
  end
rescue => e
  puts "Forwarded from `else`: #{e.message}"
end

