# typed: strict
# enable-experimental-rbs-signatures: true
# enable-experimental-rbs-assertions: true

while ARGV.any?
  break ARGV.shift #: as String
end

while ARGV.any?
  break ARGV.shift, "foo" #: Array[String]
end

while ARGV.shift
  break #: as String
  #     ^^^^^^^^^^^^ error: Unexpected RBS assertion comment found after `break`
end
