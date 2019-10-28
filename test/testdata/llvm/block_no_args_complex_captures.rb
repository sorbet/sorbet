s = "hello"
f = "outer"
10.times do
  Kernel.puts s
  d = "inner"
  1.times do 
    Kernel.puts s
    Kernel.puts f
    Kernel.puts d
  end
end
