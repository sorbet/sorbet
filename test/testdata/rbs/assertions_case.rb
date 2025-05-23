# typed: strict
# enable-experimental-rbs-comments: true

case ARGV.shift
when "foo"
  ARGV.shift #: as String
when "bar"
  ARGV.shift #: as String
end

case ARGV.shift
when "foo"
  puts "foo"
  ARGV.shift #: as String
when "bar"
  puts "bar"
  ARGV.shift #: as String
end

case ARGV.shift
when "foo"
else
  puts "else"
  ARGV.shift #: as String
end

c1 = case ARGV.shift
when "foo"
  ARGV.shift #: as String
else
  ARGV.shift #: as String
end #: as Integer
T.reveal_type(c1) # error: Revealed type: `Integer`

case ARGV.shift #: as String
when "foo"
end

case ARGV.shift
when "foo"
else ARGV.shift #: as String
end
