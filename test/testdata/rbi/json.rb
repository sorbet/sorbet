# typed: true

T.reveal_type([1, "2", :foo].to_json) # error: Revealed type: `String`
T.reveal_type(false.to_json) # error: Revealed type: `String`
T.reveal_type(1.23.to_json) # error: Revealed type: `String`
T.reveal_type({ a: 1, b: "2", "c" => :foo }.to_json) # error: Revealed type: `String`
T.reveal_type(1.to_json) # error: Revealed type: `String`
T.reveal_type(nil.to_json) # error: Revealed type: `String`
T.reveal_type(Object.new.to_json) # error: Revealed type: `String`
T.reveal_type("123".to_json) # error: Revealed type: `String`
T.reveal_type(true.to_json) # error: Revealed type: `String`
