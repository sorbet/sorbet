# frozen_string_literal: true
# typed: true
# compiled: true

begin
  begin
    raise "body"
  rescue => e
    puts "Caught: #{e.message}"
    raise "rescue"
  ensure
    puts "ensure"
  end
rescue => e
  puts "Caught: #{e.message}"
end
