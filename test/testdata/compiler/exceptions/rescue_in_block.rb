# frozen_string_literal: true
# typed: true
# compiled: true

# Test that there are not bad interactions with using exception handling inside
# of blocks.

2.times do |x|
  begin
    raise "value: #{x}"
  rescue => e
    puts e.message
  end
end

begin
  2.times do |x|
    raise "value: #{x}"
  end
rescue => e
  puts e.message
end
