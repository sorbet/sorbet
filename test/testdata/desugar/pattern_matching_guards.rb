# typed: true

x = nil

y = case nil
in 1
  1
in 2 if x
  2
in 3 unless x
  3
in 4 | 5 if x
  45
else
  6
end

T.reveal_type(y) # error: Revealed type: `Integer`
