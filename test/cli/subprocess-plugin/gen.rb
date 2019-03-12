method_name = ARGV[5].gsub("gen ", "").delete(":")
constant_name = method_name.dup
constant_name[0] = constant_name[0].upcase

puts "def #{method_name}; end"
puts "#{constant_name} = true"
