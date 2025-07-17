# typed: false

until true
  "body"
end

# empty
until true
end

# When the until loop is placed after a `begin` block, it should be a UntilPost.
begin
  puts 4
end until false

# If the begin block is assigned, the until loop should NOT be a UntilPost.
x = begin
  puts 5
end until false

# Below is the same as:
# ```rb
# foo(begin; puts 5; end) until false
# ```
foo begin
  puts 6
end until false
