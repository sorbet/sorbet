# typed: strict
# enable-experimental-rbs-comments: true

ARGV.first rescue ARGV.first #: as String

begin
  ARGV.first #: as String
rescue => e
  e #: as String
end

begin
  x = ARGV.first
  x #: as String
rescue => e
  puts e
  e #: as String
rescue => e
  puts e
  e #: as String
end

r1 = begin
  x = ARGV.first
  x #: as Integer
rescue => e
  puts e
  e #: as String
rescue => e
  puts e
  e #: as Float
end
T.reveal_type(r1) # error: Revealed type: `T.any(Integer, String, Float)`

begin
rescue => e #: as String # error: Unexpected RBS assertion comment found after `rescue`
end

begin
rescue StandardError #: as String # error: Unexpected RBS assertion comment found after `rescue`
end

begin
rescue #: as String # error: Unexpected RBS assertion comment found after `rescue`
end

def method_with_only_rescue
rescue StandardError
end
