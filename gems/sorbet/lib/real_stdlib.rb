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

  def self.real_instance_methods(mod, arg)
    @real_instance_methods ||= Module.instance_method(:instance_methods)
    @real_instance_methods.bind(mod).call(arg)
  end

  def self.real_private_instance_methods(mod, arg)
    @real_private_instance_methods ||= Module.instance_method(:private_instance_methods)
    @real_private_instance_methods.bind(mod).call(arg)
  end

  def self.real_singleton_class(obj)
    @real_singleton_class ||= Object.instance_method(:singleton_class)
    @real_singleton_class.bind(obj).call
  end

  def self.real_spaceship(obj, arg)
    @real_spaceship ||= Object.instance_method(:<=>)
    @real_spaceship.bind(obj).call(arg)
  end

  def self.real_hash(o)
    @real_hash ||= Object.instance_method(:hash)
    @real_hash.bind(o).call
  end

  def self.real_superclass(o)
    @real_superclass ||= Class.instance_method(:superclass)
    @real_superclass.bind(o).call
  end
end
