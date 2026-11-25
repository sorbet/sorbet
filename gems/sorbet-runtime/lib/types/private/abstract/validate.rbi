# typed: true

module T::Private::Abstract::Validate
  sig { params(mod: Module).void }
  def self.validate_abstract_module(mod); end

  sig { params(mod: Module).void }
  def self.validate_subclass(mod); end

  sig { params(mod: Module, method_names: T::Array[Symbol]).void }
  private_class_method def self.validate_interface_all_abstract(mod, method_names); end

  sig { params(mod: Module).void }
  private_class_method def self.validate_interface(mod); end

  sig { params(mod: Module, method_names: T::Array[Symbol]).void }
  private_class_method def self.validate_interface_all_public(mod, method_names); end

  sig { params(method: UnboundMethod, show_owner: T::Boolean).returns(String) }
  private_class_method def self.describe_method(method, show_owner: true); end
end
