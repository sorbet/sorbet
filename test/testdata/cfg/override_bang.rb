# typed: true

class C
  extend T::Sig

  sig { returns(TrueClass) }
  def !
    true
  end
end

if C.new
  puts
else
  puts # error: This code is unreachable
end

if !C.new
  puts
else
  puts # error: This code is unreachable
end

unless C.new
  puts # error: This code is unreachable
else
  puts
end

unless !C.new
  puts # error: This code is unreachable
else
  puts
end

until C.new
  puts # error: This code is unreachable
end

until !C.new
  puts # error: This code is unreachable
end
