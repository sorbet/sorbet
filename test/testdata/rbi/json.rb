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

# `JSON::Coder.new` takes a block, and accepts its options positionally (json
# 2.x) or as keyword arguments (json 3.x).
coder = JSON::Coder.new do |object|
  T.reveal_type(object) # error: Revealed type: `T.anything`
  case object
  when Time then object.iso8601
  else object
  end
end
T.reveal_type(coder) # error: Revealed type: `JSON::Coder`
JSON::Coder.new(symbolize_names: true) {}
JSON::Coder.new({ symbolize_names: true }) {}
JSON::Coder.new

# `dump` returns a `String`, or the `io` it was given.
T.reveal_type(coder.dump({ "a" => 1 })) # error: Revealed type: `String`
T.reveal_type(coder.dump({ "a" => 1 }, StringIO.new)) # error: Revealed type: `StringIO`
T.reveal_type(coder.generate({ "a" => 1 })) # error: Revealed type: `String`
T.reveal_type(coder.generate({ "a" => 1 }, StringIO.new)) # error: Revealed type: `StringIO`

# The parsed result depends on the document, so it is `T.untyped`.
T.reveal_type(coder.load('{"a":1}')) # error: Revealed type: `T.untyped`
T.reveal_type(coder.parse('{"a":1}')) # error: Revealed type: `T.untyped`
T.reveal_type(coder.load_file("example.json")) # error: Revealed type: `T.untyped`
