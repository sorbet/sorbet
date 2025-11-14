# typed: true

class Parent; end
module Mixin; end

ObjectSpace.each_object do |obj|
  T.reveal_type(obj) # error: `BasicObject`
end
ObjectSpace.each_object(Parent) do |obj|
  T.reveal_type(obj) # error: `Parent`
end
ObjectSpace.each_object(Mixin) do |obj|
  T.reveal_type(obj) # error: `Mixin`
end

res = ObjectSpace.each_object
T.reveal_type(res) # error: `T::Enumerator[BasicObject]`
res = ObjectSpace.each_object(Parent)
T.reveal_type(res) # error: `T::Enumerator[Parent]`
res = ObjectSpace.each_object(Mixin)
T.reveal_type(res) # error: `T::Enumerator[Mixin]`
