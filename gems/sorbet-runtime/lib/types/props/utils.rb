# frozen_string_literal: true
# typed: true

module T::Props::Utils
  # Deep copy an object. The object must consist of Ruby primitive
  # types and Hashes and Arrays.
  def self.deep_clone_object(what, freeze: false)
    result = case what
    when true
      true
    when false
      false
    when Symbol, NilClass, Numeric
      what
    when Array
      what.map {|v| deep_clone_object(v, freeze: freeze)}
    when Hash
      h = what.class.new
      what.each do |k, v|
        k.freeze if freeze
        h[k] = deep_clone_object(v, freeze: freeze)
      end
      h
    when Regexp
      what.dup
    else
      # Some unfortunate nastiness to get around Opus::Enum potentially not
      # being defined.
      if defined?(Opus) && defined?(Opus::Enum) && what.class == Opus::Enum
        what
      else
        what.clone
      end
    end
    freeze ? result.freeze : result
  end

  # The prop_rules indicate whether we should check for reading a nil value for the prop/field.
  # This is mostly for the compatibility check that we allow existing documents carry some nil prop/field.
  def self.need_nil_read_check?(prop_rules)
    # . :on_load allows nil read, but we need to check for the read for future writes
    prop_rules[:optional] == :on_load || prop_rules[:raise_on_nil_write]
  end

  # The prop_rules indicate whether we should check for writing a nil value for the prop/field.
  def self.need_nil_write_check?(prop_rules)
    need_nil_read_check?(prop_rules) || T::Utils::Props.required_prop?(prop_rules)
  end
end
