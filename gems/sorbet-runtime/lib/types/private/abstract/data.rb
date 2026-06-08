# frozen_string_literal: true
# typed: true

# We need to associate data with abstract modules. We could add instance methods to them that
# access ivars, but those methods will unnecessarily pollute the module namespace, and they'd
# have access to other private state and methods that they don't actually need. We also need to
# associate data with arbitrary classes/modules that implement abstract mixins, where we don't
# control the interface at all. So, we access data via these `get` and `set` methods.
#
# Using instance_variable_get/set here is gross, but the alternative is to use a hash keyed on
# `mod`, and we can't trust that arbitrary modules can be added to those, because there are lurky
# modules that override the `hash` method with something completely broken.
module T::Private::Abstract::Data
  # The in-gem key set is closed, and these accessors run on every
  # include/extend/inherit event of abstract-hooked or
  # mixes_in_class_methods modules: pre-resolve the ivar-name Symbols so
  # the per-call String interpolation (one allocation per access)
  # disappears. Novel keys from external callers fall back to
  # interpolation; the map itself is frozen (no runtime writes).
  IVAR_NAMES = {
    abstract_type: :@opus_abstract__abstract_type,
    can_have_abstract_methods: :@opus_abstract__can_have_abstract_methods,
    class_methods_mixins: :@opus_abstract__class_methods_mixins,
    last_used_by: :@opus_abstract__last_used_by,
  }.freeze
  private_constant :IVAR_NAMES

  def self.get(mod, key)
    mod.instance_variable_get(IVAR_NAMES[key] || "@opus_abstract__#{key}")
  end

  def self.set(mod, key, value)
    mod.instance_variable_set(IVAR_NAMES[key] || "@opus_abstract__#{key}", value)
  end

  def self.key?(mod, key)
    mod.instance_variable_defined?(IVAR_NAMES[key] || "@opus_abstract__#{key}")
  end

  # Works like `setdefault` in Python. If key has already been set, return its value. If not,
  # insert `key` with a value of `default` and return `default`.
  def self.set_default(mod, key, default)
    if self.key?(mod, key)
      self.get(mod, key)
    else
      self.set(mod, key, default)
      default
    end
  end
end
