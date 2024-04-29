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
