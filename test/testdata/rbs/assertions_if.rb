# typed: strict
# enable-experimental-rbs-comments: true

if ARGV.any?
  if1 = ARGV.shift #: as String
end

if ARGV.any?
  if2 = ARGV.shift #: as String
  puts if2
end

if ARGV.empty?
else
  if3 = ARGV.shift #: as String
end

if ARGV.empty?
  if4 = ARGV.shift #: as String
  puts if4
end

if5 = if ARGV.empty?
  ARGV.shift #: as String
end
T.reveal_type(if5) # error: Revealed type: `T.nilable(String)`

if6 = if ARGV.empty?
else
  ARGV.shift #: as String
end
T.reveal_type(if6) # error: Revealed type: `T.nilable(String)`

if7 = if ARGV.empty?
  ARGV.shift #: as Integer
else
  ARGV.shift #: as String
end
T.reveal_type(if7) # error: Revealed type: `T.any(Integer, String)`

if8 = if ARGV.empty?
  ARGV.shift #: as Integer
else
  ARGV.shift #: as String
end #: as Float
T.reveal_type(if8) # error: Revealed type: `Float`

if9 = if ARGV.empty? #: as Integer
else
end
