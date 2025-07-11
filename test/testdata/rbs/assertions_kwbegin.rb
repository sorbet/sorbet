# typed: strict
# enable-experimental-rbs-comments: true

begin #: as String # error: Unexpected RBS assertion comment found after `begin` declaration
end

x = begin
end #: as String

puts begin
end #: as String

begin
  42 #: String # error: Argument does not have asserted type `String`
end

begin
  42 #: String # error: Argument does not have asserted type `String`
  42 #: String # error: Argument does not have asserted type `String`
end #: as Integer
