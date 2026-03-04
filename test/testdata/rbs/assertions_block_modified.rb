# typed: strict
# enable-experimental-rbs-comments: true

#: (*untyped) {(String?) -> String} -> untyped
def take_block(*args, &); end

take_block { |x|
  x #: as String
}

take_block { |x|
  puts x
  x #: as String
}

take_block do |x|
  x #: as String
end

take_block do |x|
  puts x
  x #: as String
end

take_block do |x|
  x #: as String
end.to_s

take_block do |x|
  x #: as String
end.take_block { |x|
  x #: as String
}.to_s

take_block(
  begin
    42 #: as String
  end
) do |x|
  x #: as String
end

b1 = take_block { $1 } #: as String
T.reveal_type(b1) # error: Revealed type: `String`

b2 = take_block {
  $1
} #: as String
T.reveal_type(b2) # error: Revealed type: `String`

b3 = take_block do
   $1
end #: as String
T.reveal_type(b3) # error: Revealed type: `String`

b4 = take_block do |x|
  x #: as !nil
end #: as Integer?
T.reveal_type(b4) # error: Revealed type: `T.nilable(Integer)`

b5 ||= take_block do |x|
  x #: as !nil
end #: Integer?
T.reveal_type(b5) # error: Revealed type: `T.nilable(Integer)`

b6 = 42 #: Integer
b6 &&= take_block do |x|
  x #: as !nil
end #: String
T.reveal_type(b6) # error: Revealed type: `String`

b7 = 42 #: Integer
b7 += take_block do |x|
  x #: as !nil
end #: String # error: Expected `Integer` but found `String` for argument `arg0`
T.reveal_type(b7) # error: Revealed type: `Integer`

Object.new #: Object
  .tap {}
  .tap {}
