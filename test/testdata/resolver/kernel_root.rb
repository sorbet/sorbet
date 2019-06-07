# typed: true
module A
  fork do
    puts "OK"
  end
end
fork do
  puts "OK"
end
