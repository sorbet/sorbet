# typed: true

# The vast majority Most of the methods defined below are arguably private
# APIs, but are actually not private because they're used by Chalk::ODM.

class T::InexactStruct
  include T::Props
  include T::Props::Constructor
  include T::Props::Serializable
end

class T::Struct < T::InexactStruct
end

module T::Props
  extend T::Helpers
  mixes_in_class_methods(T::Props::ClassMethods)
end

module T::Props::ClassMethods
  sig {params(name: T.any(Symbol, String), cls_or_args: T.untyped, args: T::Hash[Symbol, T.untyped]).void}
  def const(name, cls_or_args, args={}, &blk); end
  sig {params(name: T.any(Symbol, String), cls: T.untyped, rules: T.untyped).void}
  def prop(name, cls, rules = nil); end
  def decorator; end
  def decorator_class; end
  def plugin(mod); end
  def plugins; end
  def props; end
  def reload_decorator!; end
  def validate_prop_value(prop, val); end
end

module T::Props::CustomType
  def deserialize(_mongo_scalar); end
  def instance?(_value); end
  def self.scalar_type?(val); end
  def self.valid_serialization?(val); end
  def serialize(_instance); end
  def valid?(value); end
  include Kernel
end

module T::Props::GeneratedCodeValidation
  def self.validate_serialize(source); end
  def self.validate_deserialize(source); end
end

class T::Props::Decorator
  Rules = T.type_alias {T::Hash[Symbol, T.untyped]}
  DecoratedInstance = T.type_alias {T.untyped} # Would be T::Props, but that produces circular reference errors in some circumstances
  PropType = T.type_alias {T::Types::Base}
  PropTypeOrClass = T.type_alias {T.any(PropType, Module)}
end

class T::Props::Decorator
  def add_prop_definition(prop, rules); end
  def all_props; end
  def decorated_class; end
  def foreign_prop_get(instance, prop, foreign_class, rules = {}, opts = {}); end
  def initialize(klass); end
  def model_inherited(child); end
  def plugin(mod); end
  def prop_defined(name, cls, rules = {}); end
  def prop_get_logic(instance, prop, value); end
  def prop_get(instance, prop, rules = {}); end
  def prop_get_if_set(instance, prop, rules = {}); end
  alias_method :get, :prop_get_if_set
  def prop_rules(prop); end
  def prop_set(instance, prop, value, rules = {}); end
  alias_method :set, :prop_set
  def prop_validate_definition!(name, cls, rules, type); end
  def props; end
  def self.method_added(name); end
  def self.singleton_method_added(name); end
  def valid_rule_key?(key); end
  def validate_prop_value(prop, val); end
  extend T::Sig
end

class T::Props::Decorator::NoRulesError < StandardError
end
class T::Props::Error < StandardError
end
class T::Props::InvalidValueError < T::Props::Error
end
class T::Props::ImmutableProp < T::Props::Error
end

module T::Props::Plugin
  extend T::Helpers
  include T::Props
end

module T::Props::Utils
  def self.deep_clone_object(what, freeze: nil); end
  def self.merge_serialized_optional_rule(prop_rules); end
  def self.need_nil_write_check?(prop_rules); end
  def self.optional_prop?(prop_rules); end
  def self.required_prop?(prop_rules); end
end

module T::Props::Optional
  include T::Props::Plugin
end

module T::Props::Optional::DecoratorMethods
  def add_prop_definition(prop, rules); end
  def compute_derived_rules(rules); end
  def get_default(rules, instance_class); end
  def has_default?(rules); end
  def prop_optional?(prop); end
  def prop_validate_definition!(name, cls, rules, type); end
  def valid_rule_key?(key); end
end

module T::Props::WeakConstructor
  def initialize(hash = nil); end
  include T::Props::Optional
end

module T::Props::Constructor
  def initialize(hash = nil); end
  include T::Props::WeakConstructor
end

module T::Props::PrettyPrintable
  def inspect; end
  def pretty_inspect; end
  include T::Props::Plugin
end

module T::Props::PrettyPrintable::DecoratorMethods
  def inspect_instance(instance, multiline: false, indent: '  ', &blk); end
  def inspect_instance_components(instance, multiline:, indent:, &blk); end
  def inspect_prop_value(instance, prop, multiline:, indent:, &blk); end
  def join_props_with_pretty_values(pretty_kvs, multiline:, indent: '  ', &blk); end
  def self.method_added(name); end
  def self.singleton_method_added(name); end
  def valid_rule_key?(key); end
  extend T::Sig
end

module T::Props::Serializable
  def deserialize(hash, strict = nil); end
  def recursive_stringify_keys(obj); end
  def serialize(strict = nil); end
  def with(changed_props); end
  def with_existing_hash(changed_props, existing_hash:); end
  include T::Props::Optional
  include T::Props::Plugin
  include T::Props::PrettyPrintable
  mixes_in_class_methods(T::Props::Serializable::ClassMethods)
end

module T::Props::Serializable::DecoratorMethods
  def add_prop_definition(prop, rules); end
  def extra_props(instance); end
  def from_hash(hash, strict = nil); end
  def get_id(instance); end
  def inspect_instance_components(instance, multiline:, indent:); end
  def prop_by_serialized_forms; end
  def prop_dont_store?(prop); end
  def prop_serialized_form(prop); end
  def prop_validate_definition!(name, cls, rules, type); end
  def required_props; end
  def serialized_form_prop(serialized_form); end
  def valid_rule_key?(key); end
end

module T::Props::Serializable::ClassMethods
  def from_hash!(hash); end
  def from_hash(hash, strict = nil); end
  def prop_by_serialized_forms; end
end

module T::Props::TypeValidation
  include T::Props::Plugin
end

class T::Props::TypeValidation::UnderspecifiedType < ArgumentError
end

module T::Props::TypeValidation::DecoratorMethods
  def find_invalid_subtype(*args, &blk); end
  def prop_validate_definition!(*args, &blk); end
  def self.method_added(name); end
  def self.singleton_method_added(name); end
  def type_error_message(*args, &blk); end
  def valid_rule_key?(key); end
  def validate_type(*args, &blk); end
  extend T::Sig
end

module T::Props::HasLazilySpecializedMethods
  class SourceEvaluationDisabled < RuntimeError; end
  def self.disable_lazy_evaluation!; end
end

module T::Props::GeneratedCodeValidation
  class ValidationError < RuntimeError; end
  def self.validate_deserialize(source); end
  def self.validate_serialize(source); end
end
