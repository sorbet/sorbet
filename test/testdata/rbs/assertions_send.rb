# typed: strict
# enable-experimental-rbs-comments: true

puts #: as String

ARGV.first #: as String

begin
  ARGV #: as Integer
end.first # error: Method `first` does not exist on `Integer`

puts begin
  ARGV.first #: as String
end #: as String

puts(
  42 #: as String
)

puts(
  42, #: as String
  42 #: as String
)

arr_nil = nil #: Array[untyped]?
T.unsafe(self).puts(
  *arr_nil, #: as !nil
  *arr_nil #: as !nil
)

hash_nil = nil #: Hash[untyped, untyped]?
puts(
  **hash_nil, #: as !nil
  **(
    hash_nil #: as !nil
  )
)

ARGV.
  first #: as Integer

ARGV. #: as Integer
  first #: as Integer
# ^^^^^ error: Method `first` does not exist on `Integer`

ARGV #: as Integer
 .first #: as String # error: Method `first` does not exist on `Integer`
 .last # error: Method `last` does not exist on `String`

self. #: as untyped
 attr_writer(
   ARGV.first, #: as String # error: Argument to `attr_writer` must be a Symbol or String
   ARGV.last, #: as Integer
 )

self #: as untyped
  .attr_writer #: as String
