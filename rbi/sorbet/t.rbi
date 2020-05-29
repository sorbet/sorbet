# typed: __STDLIB_INTERNAL
module T::Sig
  # We could provide a more-complete signature, but these are already
  # parsed in C++, so there's no need to emit errors twice.

  sig {params(arg0: T.nilable(Symbol), blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
  def sig(arg0=nil, &blk); end
end
module T::Sig::WithoutRuntime
  # At runtime, does nothing, but statically it is treated exactly the same
  # as T::Sig#sig. Only use it in cases where you can't use T::Sig#sig.
  sig {params(arg0: T.nilable(Symbol), blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
  def self.sig(arg0=nil, &blk); end
end

module T
  extend T::Sig

  sig {params(value: T.untyped).returns(T.untyped)}
  def self.unsafe(value); end

  # These are implemented in C++ when they appear in type context; We
  # implement them here in Ruby so they also exist if they are called
  # in value context. Several of these methods additionally have a C++
  # implementation filled in value context; In that case the prototype
  # here still applies, but additional checking and/or analysis is
  # performed in C++ for that method.

  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.let(value, type, checked: true); end

  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.assert_type!(value, type, checked: true); end

  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.cast(value, type, checked: true); end

  sig {params(type: T.untyped).returns(BasicObject)}
  def self.nilable(type); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def self.proc; end

  def self.class_of(klass); end
  def self.noreturn; end
  def self.enum(values); end
  def self.untyped; end

  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.any(type_a, type_b, *types); end

  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.all(type_a, type_b, *types); end

  def self.reveal_type(value); end
  def self.type_parameter(name); end
  def self.self_type; end
  def self.attached_class; end
  def self.type_alias(type=nil, &blk); end

  sig {params(arg: T.untyped).returns(T.untyped)}
  def self.must(arg); end

  def self.coerce(type); end

  sig {params(value: BasicObject).returns(T.noreturn)}
  def self.absurd(value); end
end

module T::Generic
  include T::Helpers

  sig {params(params: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def type_parameters(*params); end

  def type_member(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject); end
  def type_template(variance=:invariant, fixed: nil, lower: T.untyped, upper: BasicObject); end
  def [](*types); end
end

module T::Helpers
  sig {void}
  def abstract!;  end
  sig {void}
  def interface!; end
  sig {void}
  def final!; end
  sig {void}
  def sealed!; end
  def mixes_in_class_methods(mod); end
end

module T::Array
  def self.[](type); end
end
module T::Hash
  def self.[](keys, values); end
end
module T::Set
  def self.[](type); end
end
module T::Range
  def self.[](type); end
end
module T::Enumerable
  def self.[](type); end
end
module T::Enumerator
  def self.[](type); end
end

T::Boolean = T.type_alias {T.any(TrueClass, FalseClass)}

module T::Configuration
  def self.call_validation_error_handler(signature, opts); end
  def self.call_validation_error_handler=(value); end
  def self.default_checked_level=(default_checked_level); end
  def self.enable_checking_for_sigs_marked_checked_tests; end
  def self.enable_final_checks_on_hooks; end
  def self.enable_legacy_t_enum_migration_mode; end
  def self.reset_final_checks_on_hooks; end
  def self.hard_assert_handler(str, extra); end
  def self.hard_assert_handler=(value); end
  def self.inline_type_error_handler(error); end
  def self.inline_type_error_handler=(value); end
  def self.log_info_handler(str, extra); end
  def self.log_info_handler=(value); end
  def self.scalar_types; end
  def self.scalar_types=(values); end
  def self.sealed_violation_whitelist; end
  def self.sealed_violation_whitelist=(sealed_violation_whitelist); end
  def self.sig_builder_error_handler(error, location); end
  def self.sig_builder_error_handler=(value); end
  def self.sig_validation_error_handler(error, opts); end
  def self.sig_validation_error_handler=(value); end
  def self.soft_assert_handler(str, extra); end
  def self.soft_assert_handler=(value); end
end

module T::Profile
  def self.reset; end
  def self.typecheck_count_estimate; end
  def self.typecheck_duration; end
  def self.typecheck_duration=(arg0); end
  def self.typecheck_duration_estimate; end
  def self.typecheck_sample_attempts; end
  def self.typecheck_sample_attempts=(arg0); end
  def self.typecheck_samples; end
  def self.typecheck_samples=(arg0); end
end

module T::Utils
  def self.arity(method); end
  def self.coerce(val); end
  def self.resolve_alias(type); end
  def self.run_all_sig_blocks; end
  def self.signature_for_instance_method(mod, method_name); end
  def self.unwrap_nilable(type); end
  def self.wrap_method_with_call_validation_if_needed(mod, method_sig, original_method); end

  # only one caller; delete
  def self.methods_excluding_object(mod); end
  # only one caller; delete
  def self.register_forwarder(from_method, to_method, remove_first_param: nil); end
end

class T::Utils::RuntimeProfiled
end

module T::AbstractUtils
  def self.abstract_method?(method); end
  def self.abstract_methods_for(mod); end
  def self.abstract_module?(mod); end
  def self.declared_abstract_methods_for(mod); end
end

class T::InterfaceWrapper
  def self.dynamic_cast(obj, mod); end
end

module T::Utils::Nilable
  def self.get_type_info(prop_type); end
  def self.get_underlying_type(prop_type); end
  def self.get_underlying_type_object(prop_type); end
end

module T::NonForcingConstants
  # See <https://sorbet.org/docs/non-forcing-constants> for full docs.
  sig {params(val: BasicObject, klass: String).returns(T::Boolean)}
  def self.non_forcing_is_a?(val, klass); end
end
