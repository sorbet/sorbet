# typed: strict

begin
  x = 1
  while true
    raise
  end
rescue
  T.reveal_type(x) # error: `T.nilable(Integer)`
end
