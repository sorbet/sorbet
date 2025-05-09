# typed: strict
# enable-experimental-rbs-comments: true

while ARGV.any?
  break ARGV.shift #: as String
end

while ARGV.any?
  break(
    ARGV.shift #: as String
  )
end

while ARGV.any?
  break ARGV.shift, "foo" #: Array[String]
end

while ARGV.any?
  break ARGV.shift, #: as String
   "foo" #: String
end

while ARGV.shift
  break #: as String
  #     ^^^^^^^^^^^^ error: Unexpected RBS assertion comment found after `break`
end
