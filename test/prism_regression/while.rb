# typed: false

while foo
  5
end

# empty
while true
end

# multi-statement
while true
  x = 3 + 2
  puts "hi"
end

# When the while loop is placed after a `begin` block, it should be a WhilePost.
begin
  puts 4
end while false

# If the begin block is assigned, the while loop should NOT be a WhilePost.
x = begin
  puts 5
end while false

# Below is the same as:
# ```rb
# foo(begin; puts 5; end) while false
# ```
foo begin
  puts 6
end while false
