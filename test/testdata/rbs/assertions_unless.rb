# typed: strict
# enable-experimental-rbs-comments: true

unless ARGV.any?
  ARGV.shift #: as String
end

unless ARGV.any?
  unless2 = ARGV.shift #: as String
  puts unless2
end

unless ARGV.empty?
else
  ARGV.shift #: as String
end

unless ARGV.empty?
  unless4 = ARGV.shift #: as String
  puts unless4
end

unless5 = unless ARGV.empty?
  ARGV.shift #: as String
end
T.reveal_type(unless5) # error: Revealed type: `T.nilable(String)`

unless6 = unless ARGV.empty?
else
  ARGV.shift #: as String
end
T.reveal_type(unless6) # error: Revealed type: `T.nilable(String)`

unless7 = unless ARGV.empty?
  ARGV.shift #: as Integer
else
  ARGV.shift #: as String
end
T.reveal_type(unless7) # error: Revealed type: `T.any(String, Integer)`

unless8 = unless ARGV.empty?
  ARGV.shift #: as Integer
else
  ARGV.shift #: as String
end #: as Float
T.reveal_type(unless8) # error: Revealed type: `Float`

unless9 = unless ARGV.empty? #: as Integer
else
end
