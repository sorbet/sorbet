# typed: true

# These things are things that Sorbet considers unstable internal APIs that
# could (and do) change at any moment without notice.
#
# Use them at your own risk.

module T::Private::Types
end

class T::Private::Types::Void < T::Types::Base
end

module T::Private::Methods
  def self.signature_for_method(method); end
end

module T::Private::Methods::CallValidation
  def self.disable_fast_path; end
end

module T::Private::Compiler
  sig {returns(T::Boolean)}
  def self.running_compiled?; end

  sig {returns(T.nilable(String))}
  def self.compiler_version; end
end

class T::Private::Methods::Signature
  def method; end
  def method_name; end
  def arg_types; end
  def kwarg_types; end
  def block_type; end
  def block_name; end
  def rest_type; end
  def rest_name; end
  def keyrest_type; end
  def keyrest_name; end
  def bind; end
  def return_type; end
  def mode; end
  def req_arg_count; end
  def req_kwarg_names; end
  def has_rest; end
  def has_keyrest; end
  def check_level; end
  def parameters; end
  def on_failure; end
  def override_allow_incompatible; end
  def defined_raw; end

  def self.new_untyped(method:, mode: T.unsafe(nil), parameters: T.unsafe(nil)); end

  def initialize(
    method:,
    method_name:,
    raw_arg_types:,
    raw_return_type:,
    bind:,
    mode:,
    check_level:,
    on_failure:,
    parameters: method.parameters,
    override_allow_incompatible: false,
    defined_raw: false)
  end

  def as_alias(alias_name); end

  def arg_count; end

  def kwarg_names; end

  def owner; end

  def dsl_method; end

  def each_args_value_type(args, &blk); end

  def method_desc; end
end
