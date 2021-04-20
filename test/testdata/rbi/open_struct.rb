# typed: strict

require 'ostruct'

OpenStruct.new
person = OpenStruct.new(name: 'John Smith', age: 70)
person.send(:age)
person[:name]
person[:not_defined]
person[:name] = 'Jane Smith'

T.reveal_type(person.send(:age)) # error: Revealed type: `T.untyped`
T.reveal_type(person[:name]) # error: Revealed type: `T.untyped`
T.reveal_type(person[:not_defined]) # error: Revealed type: `T.untyped`

T.assert_type!(person.hash, Integer)
T.assert_type!(person == person, T::Boolean)

T.assert_type!(person.each_pair, T::Enumerator[[Symbol,T.untyped]])

person.each_pair do |k, v|
  T.reveal_type(k) # error: Revealed type: `Symbol`
  T.reveal_type(v) # error: Revealed type: `T.untyped`
end

T.assert_type!(person.to_h, T::Hash[Symbol, T.untyped])
