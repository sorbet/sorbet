# typed: strict
# enable-experimental-rbs-comments: true

while ARGV.any?
  ARGV.shift #: as String
end

while ARGV.any?
  x = ARGV.shift #: String?
  puts x
  x #: as !nil
end

y = while ARGV.any?
  ARGV.shift #: as String
end #: String?
T.reveal_type(y) # error: Revealed type: `T.nilable(String)`

while ARGV.any?
end #: String?

while ARGV.shift #: as !nil # error: `T.must` called on `T.untyped`, which is redundant
end

begin
  ARGV.shift #: as String
end while ARGV.any?

begin
end while ARGV.shift #: as !nil # error: `T.must` called on `T.untyped`, which is redundant
