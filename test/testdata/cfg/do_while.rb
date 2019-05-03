# typed: true

# These are a do-while loops. The insides are reachable.

begin
  puts 2
end while false

begin
  puts 2
end until true

# These are not do-while loops because the body is not a Kwbegin node.
# (check the parse-tree for clarification)

x = 0
x = begin
  puts 2 # error: This code is unreachable
  1
end while false

y = 0
y = begin
  puts 2 # error: This code is unreachable
  1
end until true

puts x, y
