# compiled: true
# typed: strict

class Chalk::ODM::Document
  include T::Props
  include T::Props::Serializable
  include T::Props::WeakConstructor
  extend T::Helpers
  abstract!
end

class MyParentDocument < Chalk::ODM::Document
  prop :my_parent_method, String
end

class MyChildDocument < MyParentDocument
  prop :my_child_method, Integer
end

# Uses T::Props::WeakConstructor, so these should be ok
MyParentDocument.new
MyChildDocument.new

p MyParentDocument.new(my_parent_method: 'hello').my_parent_method
p MyChildDocument.new(my_child_method: 42).my_child_method
