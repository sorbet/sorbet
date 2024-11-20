# typed: false

BEGIN {
  puts "hi"
}

U = redo

def foo
  if 0..
    puts "IFlipflop"
  end

  if 0...
    puts "EFlipflop"
  end
end

END {
  puts "bye"
}
