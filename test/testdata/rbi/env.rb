# typed: strict

r1 = ENV.merge!
T.reveal_type(r1) # error: Revealed type: `Sorbet::Private::Static::ENVClass`

r2 = ENV.merge!({foo: "bar"})
T.reveal_type(r2) # error: Revealed type: `Sorbet::Private::Static::ENVClass`

r3 = ENV.merge!({foo: nil}) do |name, env_value, hash_value|
  T.reveal_type(name) # error: Revealed type: `String`
  T.reveal_type(env_value) # error: Revealed type: `T.nilable(String)`
  T.reveal_type(hash_value) # error: Revealed type: `T.nilable(String)`
  "baz"
end
T.reveal_type(r3) # error: Revealed type: `Sorbet::Private::Static::ENVClass`
