# typed: true
s = "hello"
f = "outer"
mut = 1
10.times do
  Kernel.puts s
  d = "inner"
  1.times do 
    Kernel.puts s
    Kernel.puts f
    Kernel.puts d
    mut = 0
  end
end

puts mut
