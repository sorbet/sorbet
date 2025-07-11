# typed: strict
# enable-experimental-rbs-comments: true

until ARGV.empty?
  ARGV.shift #: as String
end

until ARGV.empty?
  x = ARGV.shift #: String?
  puts x
  x #: as !nil
end

y = until ARGV.empty?
  ARGV.shift #: as String
end #: String?
T.reveal_type(y) # error: Revealed type: `T.nilable(String)`

until ARGV.empty?
end #: String?

until ARGV.shift #: as String
end

begin
  ARGV.shift #: as String
end until ARGV.any?

begin
end until ARGV.shift #: as !nil # error: `T.must` called on `T.untyped`, which is redundant
