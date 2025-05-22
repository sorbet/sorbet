# typed: strict
# enable-experimental-rbs-comments: true

begin
  ARGV.first #: as Integer
ensure
  ARGV.first #: as String
end
