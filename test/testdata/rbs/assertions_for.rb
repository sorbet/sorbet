# typed: strict
# enable-experimental-rbs-signatures: true
# enable-experimental-rbs-assertions: true

for x in ARGV
  x #: as String
end

for x in ARGV
  x #: as String
  puts x
end

for x in ARGV #: Array[String]
end

for x in ARGV
end #: as String

y = for x in ARGV
end #: as String
