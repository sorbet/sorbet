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
  puts # error: This code is unreachable
  # TODO this should be reachable
else
  puts # error: This code is unreachable
end

unless C.new
  #    ^ error: This code is unreachable
  # TODO this C.new should be reachable
  puts # error: This code is unreachable
else
  puts # error: This code is unreachable
  # TODO this should be reachable
end

unless !C.new
  #     ^ error: This code is unreachable
  # TODO this C.new should be reachable
  puts # error: This code is unreachable
else
  puts # error: This code is unreachable
end

until C.new
  #   ^ error: This code is unreachable
  # TODO this C.new should be reachable
  puts # error: This code is unreachable
end

until !C.new
  #    ^ error: This code is unreachable
  # TODO this C.new should be reachable
  puts # error: This code is unreachable
end
