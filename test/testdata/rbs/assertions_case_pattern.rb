# typed: strict
# enable-experimental-rbs-comments: true

case ARGV.first
in String
  ARGV.first #: as String
in Integer
  ARGV.first #: as Integer
else
  ARGV.first #: as untyped
end
