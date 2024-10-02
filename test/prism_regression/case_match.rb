# typed: false

case foo
in 1
  puts "one!"
in 2
  puts "two!"
else
  puts "Who knows!"
end

# no else
case foo
in 1
  "one!"
  puts "surprise, multi-line!"
end
