# typed: true

aaaa = 0
aaaa =
  if rand(2) == 0
    ''
  end

T.reveal_type(aaaa) # error: T.nilable(String)

bbbb = 0
bbbb =
  if rand(2) == 0
    ''
  elsif true
    ''
  end

T.reveal_type(bbbb) # error: `String("")`
