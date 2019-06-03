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
  puts # TODO this puts should be unreachable
end

until !C.new
  #    ^ error: This code is unreachable
  # TODO this C.new should be reachable
  puts # error: This code is unreachable
end
