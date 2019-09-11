# typed: true

class Array
  include JSON::Ext::Generator::GeneratorMethods::Array
end

class FalseClass
  include JSON::Ext::Generator::GeneratorMethods::FalseClass
end

class Float
  include JSON::Ext::Generator::GeneratorMethods::Float
end

class Hash
  include JSON::Ext::Generator::GeneratorMethods::Hash
end

class Integer
  include JSON::Ext::Generator::GeneratorMethods::Integer
end

class NilClass
  include JSON::Ext::Generator::GeneratorMethods::NilClass
end

class Object
  include JSON::Ext::Generator::GeneratorMethods::Object
end

class String
  include JSON::Ext::Generator::GeneratorMethods::String
end

class TrueClass
  include JSON::Ext::Generator::GeneratorMethods::TrueClass
end

T.reveal_type([1, "2", :foo].to_json) # error: Revealed type: `String`
T.reveal_type(false.to_json) # error: Revealed type: `String`
T.reveal_type(1.23.to_json) # error: Revealed type: `String`
T.reveal_type({ a: 1, b: "2", "c" => :foo }.to_json) # error: Revealed type: `String`
T.reveal_type(1.to_json) # error: Revealed type: `String`
T.reveal_type(nil.to_json) # error: Revealed type: `String`
T.reveal_type(Object.new.to_json) # error: Revealed type: `String`
T.reveal_type("123".to_json) # error: Revealed type: `String`
T.reveal_type(true.to_json) # error: Revealed type: `String`
