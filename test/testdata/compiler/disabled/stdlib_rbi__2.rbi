# frozen_string_literal: true
# typed: true
class Array
  include ::JSON::Ext::Generator::GeneratorMethods::Array
  def to_h; end
end
