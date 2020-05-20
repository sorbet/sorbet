# frozen_string_literal: true
# typed: true
# compiled: true

# Since we rely on relative rubyBlockId numbering for detecting exception
# handling, this ensures that we're not basing it on the value of the outer-most
# rubyBlockId.

begin
  raise "first"
rescue => e
  puts e.message
end

begin
  raise "second"
rescue => e
  puts e.message
end
