# frozen_string_literal: true
# typed: true

# Calling a method on a user's module caches the lookup in that module's own call cache
# table, which lives as long as the module does. sorbet-runtime reflects on every module
# that defines a method with a sig, so it calls these through `bind_call` instead to avoid
# leaving a cache entry behind in each of them.
module T::Private::UnboundMethods
  EQUAL = BasicObject.instance_method(:equal?)

  INSTANCE_VARIABLE_DEFINED = Kernel.instance_method(:instance_variable_defined?)
  OBJECT_ID = Kernel.instance_method(:object_id)
  SINGLETON_CLASS = Kernel.instance_method(:singleton_class)

  ALIAS_METHOD = Module.instance_method(:alias_method)
  DEFINE_METHOD = Module.instance_method(:define_method)
  INSTANCE_METHOD = Module.instance_method(:instance_method)
  METHOD_DEFINED = Module.instance_method(:method_defined?)
  MODULE_EXEC = Module.instance_method(:module_exec)
  PRIVATE_METHOD_DEFINED = Module.instance_method(:private_method_defined?)
  PROTECTED_METHOD_DEFINED = Module.instance_method(:protected_method_defined?)
  PUBLIC_METHOD_DEFINED = Module.instance_method(:public_method_defined?)
  RUBY2_KEYWORDS = Module.instance_method(:ruby2_keywords)

  VISIBILITY = {
    public: Module.instance_method(:public),
    protected: Module.instance_method(:protected),
    private: Module.instance_method(:private),
  }.freeze
end
