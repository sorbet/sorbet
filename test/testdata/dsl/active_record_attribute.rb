# typed: true
# enable_dsl: active_record_attribute.json

class SomeModel < ActiveRecord::Base
#                 ^^^^^^^^^^^^ error: Unable to resolve constant `ActiveRecord`
  extend T::Sig

  attribute :some_id, ActiveRecord::Type::String.new

  def assign
    self.some_id = 100
#   ^^^^^^^^^^^^^^^^^^ error: Assigning a value to `new_value` that does not match expected type `String`
  end
end
