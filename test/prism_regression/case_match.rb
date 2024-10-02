# typed: false

case foo
in 1
  puts "one!"
in 2
  puts "two!"
else
  puts "Who knows!"
end

case array_like_thing
in []
  puts "empty!"
in [1, 2]
  puts "one and two!"
in 3, 4 # An array pattern without [], but otherwise similar to the one above
  puts "three and four!"
in [5, *]
  puts "starts with five!"
in [*, 6]
  puts "ends with six!"
in [*, 7, *] # A "find pattern"
  puts "contains a seven!"
end

# no else
case foo
in 1
  "one!"
  puts "surprise, multi-line!"
end
