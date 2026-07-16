# typed: true

r = Ractor.new("hello") do |msg|
  msg
end
T.reveal_type(r) # error: Revealed type: `Ractor`

r2 = Ractor.new do
  42
end
T.reveal_type(r2) # error: Revealed type: `Ractor`

r3 = Ractor.new("a", "b") do |a, b|
  a + b
end
T.reveal_type(r3) # error: Revealed type: `Ractor`

r4 = Ractor.new(name: "test") do
  42
end
T.reveal_type(r4) # error: Revealed type: `Ractor`
