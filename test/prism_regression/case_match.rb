# typed: false

case foo
in 1
  puts "one!"
in 2
  puts "two!"
in 3 | 4
  puts "three or four!"
else
  puts "Who knows!"
end

case array_like_thing
in [] => a
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
in Array[] # Empty ArrayPattern inside a ConstPattern
  puts "empty!"
in Array[first, second] # Requires the `array_like_thing` to be an `Array` specifically
  puts "An Array with first: #{first} and second: #{second}"
in Point[x, y]          # Requires the `array_like_thing` to be a `Point` specifically
  puts "A Point with x: #{x} and y: #{y}"
in [i,]
  puts "An array with an element and maybe other stuff"
end

case hash_like_thing
in {}
  puts "empty!"
in { a: 1, b: 2 } => h
  puts "#{h} contains a and b, and maybe other stuff!"
in { c: 3, ** } => h
  puts "#{h} has c, and maybe other stuff!"
in { d: 4, **nil } => h
  puts "#{h} has d and nothing else!"
in {"kj": j} | {"kh": l} => m
  puts "#{m} has j or l!"
in {"n1":, n2:, "n3":} => n4
  puts "#{n4} has n1, n2, and n3!"
in Hash[] # This is still an empty ArrayPattern inside a ConstPattern, regardless of the constant being `Hash`.
  puts "empty!"
in Hash[e: 5 => e]             # Requires the `hash_like_thing` to be a `Hash` specifically
  puts "A Hash with e: #{e}"
in Point[x: 6 => x, y: 7 => y] # Requires the `hash_like_thing` to be a `Point` specifically
  puts "A Point with x: #{x} and y: #{y}"
in **o
  puts "splat!"
in **nil
  puts "splat nil!"
end

# no else
case foo
in 1
  "one!"
  puts "surprise, multi-line!"
end

# pattern matching with if guards
case bar
in x if x == 1
  "in with if"
in a, b if b == 2
  "in with 2 args and if"
in c, d; c if c == 3
  "in with 2 args, semicolon, and if"
end

# pattern matching with unless guards
case baz
in x unless x == 1
  "in with unless"
in a, b unless b == 2
  "in with 2 args and unless"
in c, d; c unless c == 3
  "in with 2 args, semicolon, and unless"
end

# TODO: test ConstPattern with `()`, e.g. `Constant(1, 2)` and `Constant(k: v)`
