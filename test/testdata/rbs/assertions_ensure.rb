# typed: strict
# enable-experimental-rbs-comments: true

begin
  ARGV.first #: as Integer
ensure
  ARGV.first #: as String
end

def method_with_only_ensure # error: The method `method_with_only_ensure` does not have a `sig`
ensure
end
