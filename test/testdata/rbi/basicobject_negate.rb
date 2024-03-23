# typed: true

if !nil # if true
  puts "true branch"
else
  puts "else branch" # error: This code is unreachable
end

a = T.let(nil, BasicObject)

if !a
  puts "true branch"
else
  puts "else branch"
end

if !false # if true
  puts "true branch"
else
  puts "else branch" # error: This code is unreachable
end

if !true # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if !0 # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if ![] # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

if !"" # if false
  puts "true branch" # error: This code is unreachable
else
  puts "else branch"
end

class A; end

if !A.new # if false
  puts "true branch"
else
  puts "else branch"
end
