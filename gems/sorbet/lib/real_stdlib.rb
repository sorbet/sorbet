# frozen_string_literal: true
# typed: true

module Sorbet::Private::RealStdlib
  def self.real_is_a?(o, klass)
    @real_is_a ||= Object.instance_method(:is_a?)
    @real_is_a.bind(o).call(klass)
  end

  def self.real_constants(mod)
    @real_constants ||= Module.instance_method(:constants)
    @real_constants.bind(mod).call(false)
  end

  def self.real_object_id(o)
    @real_object_id ||= Object.instance_method(:object_id)
    @real_object_id.bind(o).call
  end

  def self.real_name(o)
    @real_name ||= Module.instance_method(:name)
    @real_name.bind(o).call
  end

  def self.real_ancestors(mod)
    @real_ancestors ||= Module.instance_method(:ancestors)
    @real_ancestors.bind(mod).call
  end
end
