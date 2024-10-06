# typed: true

aaaa = 0
aaaa =
  if rand(2) == 0
    ''
  end

T.reveal_type(aaaa) # error: T.nilable(String)
