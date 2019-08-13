# frozen_string_literal: true
# typed: true

class T::Private::DeclState
  def self.current
    Thread.current[:opus_types__decl_state] ||= self.new
  end

  def self.current=(other)
    Thread.current[:opus_types__decl_state] = other
  end

  attr_accessor :active_declaration
  attr_accessor :skip_on_method_added

  def reset!
    self.active_declaration = nil
  end

  def self.without_on_method_added
    begin
      old_value = current.skip_on_method_added
      current.skip_on_method_added = true
      yield
    ensure
      current.skip_on_method_added = old_value
    end
    nil
  end
end
