# typed: true

aaaa = 0
aaaa =
  if rand(2) == 0
    ''
  end

T.reveal_type(aaaa) # error: T.any(Integer, String)

bbbb = 0
bbbb =
  if rand(2) == 0
    ''
  elsif true
    ''
  end # error: This code is unreachable

T.reveal_type(bbbb) # error: `T.any(Integer, String)`
