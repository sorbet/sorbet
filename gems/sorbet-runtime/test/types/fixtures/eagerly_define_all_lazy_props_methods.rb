# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that ObjectSpace iteration
# is isolated from the main test process.

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

class MyStruct < T::Struct
  prop :name, String
  prop :age, Integer
end

check("lazy methods pending before") do
  # private method call
  !MyStruct.decorator.send(:lazily_defined_methods).empty?
end

T::Utils.eagerly_define_all_lazy_props_methods!

check("lazy methods resolved after") do
  # private method call
  MyStruct.decorator.send(:lazily_defined_methods).empty?
end

# After eager definition, serialization should still work correctly
instance = MyStruct.new(name: "Alice", age: 30)
serialized = instance.serialize

check("serialize produces correct hash") { serialized == {"name" => "Alice", "age" => 30} }

deserialized = MyStruct.from_hash(serialized)
check("from_hash round-trips correctly") { deserialized.name == "Alice" && deserialized.age == 30 }

puts "PASS"
