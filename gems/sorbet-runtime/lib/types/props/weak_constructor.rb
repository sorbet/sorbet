# frozen_string_literal: true
# typed: false

module T::Props::WeakConstructor
  include T::Props::Optional

  def initialize(hash={})
    expected_keys = {}
    hash.each_key {|key| expected_keys[key] = true}

    decorator = self.class.decorator

    decorator.props.each do |p, rules|
      if hash.key?(p)
        expected_keys.delete(p)
        val = hash[p]
      elsif decorator.has_default?(rules)
        val = decorator.get_default(rules, self.class)
      else
        next
      end

      decorator.prop_set(self, p, val, rules)
    end

    unless expected_keys.empty?
      raise ArgumentError.new("#{@class}: Unrecognized properties in #with_props: #{expected_keys.keys.join(', ')}")
    end
  end
end
