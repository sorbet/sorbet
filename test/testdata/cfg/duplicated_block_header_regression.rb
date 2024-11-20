# typed: true
extend T::Sig

xyz = 0.to_i
found = T.let(true, T::Boolean)

while found
  found = false
  if [true, false].sample
    found = true
  end
end

T.reveal_type(xyz) # error: Revealed type: `Integer`
