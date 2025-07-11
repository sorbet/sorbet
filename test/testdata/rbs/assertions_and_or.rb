# typed: strict
# enable-experimental-rbs-comments: true

begin
  ARGV.first #: as String
end && begin
  ARGV.first #: as String
end

begin
  ARGV.first #: as String
end and begin
  ARGV.first #: as String
end

begin
  ARGV.first #: as String
  ARGV.empty?
end || begin
  ARGV.first #: as String
end

begin
  ARGV.first #: as String
  ARGV.empty?
end or begin
  ARGV.first #: as String
end

x = ARGV.first || ARGV.last #: as String

ARGV.first || ARGV.last #: as String

y = ARGV.first && ARGV.last #: as String

ARGV.first && ARGV.last #: as String

(
  ARGV.first #: as String?
) || (
  ARGV.last #: as String
)

ARGV.first || ( #: as String? # error: Unexpected RBS assertion comment found after `begin`
  ARGV.last #: as String
)

ARGV.first && ( #: as String # error: Unexpected RBS assertion comment found after `begin`
  ARGV.last #: as String
)
