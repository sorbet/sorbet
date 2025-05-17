# typed: strict
# enable-experimental-rbs-comments: true

while ARGV.any?
  next ARGV.shift #: as String
end

while ARGV.any?
  next ARGV.shift, "foo" #: Array[String]
end

while ARGV.any?
  next ARGV.shift, #: as String
   "foo" #: String
end

while ARGV.shift
  next #: as String
  #    ^^^^^^^^^^^^ error: Unexpected RBS assertion comment found after `next`
end
