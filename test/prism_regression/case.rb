# typed: false

def foo; end

case foo
when 1
  puts "one!"
when 2, 3
  puts "two or three!"
else
  puts "Who knows!"
end

# no else
case foo
when Integer
  4
  puts "surprise, multi-line!"
end
