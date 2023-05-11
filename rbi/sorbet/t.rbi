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

  # Statically, converts the argument to type `T.untyped`.
  # This is mostly useful to silence type errors.
  #
  # At runtime, returns the provided argument completely unchanged.
  #
  # For more, see https://sorbet.org/docs/troubleshooting#escape-hatches
  sig {params(value: T.untyped).returns(T.untyped)}
  def self.unsafe(value); end

  # These are implemented in C++ when they appear in type context; We
  # implement them here in Ruby so they also exist if they are called
  # in value context. Several of these methods additionally have a C++
  # implementation filled in value context; In that case the prototype
  # here still applies, but additional checking and/or analysis is
  # performed in C++ for that method.

  # Sorbet's syntax for a type annotation.
  #
  # Statically, asserts that the value has the specified type, reporting a type
  # error if the asserted type is incorrect. Sorbet then assumes that the
  # variable has this type throughout the rest of the program. Can be used to
  # explicitly declare a variable's type or to widen an inferred type, which
  # are sometimes required.
  #
  # At runtime, raises an exception if the value does not have the asserted
  # type.
  #
  # For more information, see both of these docs:
  #
  # - https://sorbet.org/docs/type-annotations
  # - https://sorbet.org/docs/type-assertions
  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.let(value, type, checked: true); end

  # A special form of `T.cast` that can be used to do an unchecked cast on the
  # `self` keyword. Useful when the type of `self` at runtime is not the same
  # as the type of `self` that Sorbet can infer from the lexically enclosing
  # scope.
  #
  # For more information, see https://sorbet.org/docs/type-assertions#tbind
  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.bind(value, type, checked: true); end

  # Similar to `T.let`, but fails if the argument is ever `T.untyped` (`T.let`
  # would simply say that `T.untyped` is a subtype of all other types, and thus
  # trivially pass the type assertion).
  #
  # Useful to ensure that a declared type does not regress to `T.untyped` as a
  # result of some refactoring.
  #
  # For more information, see https://sorbet.org/docs/type-assertions#tassert_type
  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.assert_type!(value, type, checked: true); end

  # Statically, instructs Sorbet to unconditionally assume that the value has
  # the asserted type. The type is not checked statically--Sorbet merely
  # assumes that the casted type is correct. This is considered an escape
  # hatch, as it allows breaking out of the constraints Sorbet normally imposes
  # on valid programs.
  #
  # At runtime, raises an exception if the value does not have the asserted
  # type.
  #
  # For more information, see https://sorbet.org/docs/type-assertions#tcast
  sig {params(value: T.untyped, type: T.untyped, checked: T::Boolean).returns(BasicObject)}
  def self.cast(value, type, checked: true); end

  # Type syntax to declare that a type may include the `nil` value.
  #
  # `T.nilable(...)` is indistinguishable from `T.any(NilClass, ...)`, and is thus an example of a union type.
  #
  # For more information, see both of:
  #
  # - https://sorbet.org/docs/nilable-types
  # - https://sorbet.org/docs/union-types
  sig {params(type: T.untyped).returns(BasicObject)}
  def self.nilable(type); end

  # Type syntax to declare the type of a block, proc, or lambda.
  # Shares much of the same syntax with `sig` syntax for methods.
  #
  # For more information, see https://sorbet.org/docs/procs
  sig {returns(T::Private::Methods::DeclBuilder)}
  def self.proc; end

  # Type syntax to declare the type of a class's singleton class.
  #
  # For example:
  #
  # - the value `0` has type `Integer`
  # - the value `Integer` has type `T.class_of(Integer)`
  #
  # (because classes in Ruby are proper values in their own right)
  #
  # For more information, see https://sorbet.org/docs/class-of
  def self.class_of(klass); end

  # Type syntax to declare the "bottom" type in Sorbet. Put another way,
  # declares that a method always raises (or somehow else does not return).
  # Plays a key part in powering Sorbet's support for dead code.
  #
  # For more information, see https://sorbet.org/docs/noreturn
  def self.noreturn; end

  # Type syntax to declare the "top" type in Sorbet. Every type is a subtype of
  # this type, but absolutely nothing is known about values of this type.
  def self.anything; end

  # Deprecated. Use `T::Enum` instead.
  #
  # For more information, see https://sorbet.org/docs/tenum
  def self.deprecated_enum(values); end

  # Type syntax to opt out of static type checking.
  #
  # For more information, see both of:
  #
  # - https://sorbet.org/docs/gradual
  # - https://sorbet.org/docs/untyped
  def self.untyped; end

  # Type syntax to declare a union type, which declares that a value has either
  # one type or another type.
  #
  # For more information, see https://sorbet.org/docs/union-types
  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.any(type_a, type_b, *types); end

  # Type syntax to declare an intersection type, which declares that a value
  # has both one type and another type. For example, a value might implement
  # one interface type and a second, unrelated interface type.
  #
  # For more information, see https://sorbet.org/docs/intersection-types
  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.all(type_a, type_b, *types); end

  # Utility to request that Sorbet report a static error showing the type of
  # the wrapped expression.
  #
  # Most useful for troubleshooting why Sorbet is behaving in a certain way.
  #
  # For more information, see https://sorbet.org/docs/troubleshooting
  def self.reveal_type(value); end

  # Type syntax to declare a generic type variable scoped to a single method.
  #
  # For more information, see https://sorbet.org/docs/generics#generic-methods
  def self.type_parameter(name); end

  # Type syntax to declare a type equivalent to the receiver of the method
  # (i.e., the type of the `self` keyword).
  #
  # For more information, see https://sorbet.org/docs/self-type
  def self.self_type; end

  # Type syntax to declare the type that instances of the current singleton
  # class would have. Most useful for annotating the result type of factory
  # methods, which construct instances of a given class.
  #
  # For more information, see https://sorbet.org/docs/attached-class
  def self.attached_class; end

  # Syntax for declaring type aliases, or alternative shorthand names for
  # longer types.
  #
  # For more information, see https://sorbet.org/docs/type-aliases
  def self.type_alias(type=nil, &blk); end

  # Statically, declares to Sorbet that the argument is never `nil`, despite
  # what the type system would otherwise infer for the type.
  #
  # At runtime, raises an exception if the argument is ever `nil`.
  #
  # To provide a custom message on failure, use `T.must_because`.
  #
  # For more, see https://sorbet.org/docs/type-assertions#tmust
  sig {params(arg: T.untyped).returns(T.untyped)}
  def self.must(arg); end

  # Statically, declares to Sorbet that the argument is never `nil`, despite
  # what the type system would otherwise infer for the type.
  #
  # At runtime, raises an exception contining the provided reason if the
  # argument is ever `nil`.
  #
  # Takes the reason as a block that should return a `String`, so that the code
  # to compute the reason is only run if there is a problem, and doesn't slow
  # down well-behaved code.
  #
  # For more, see https://sorbet.org/docs/type-assertions#tmust_because
  sig {params(arg: T.untyped, reason_blk: T.proc.returns(String)).returns(T.untyped)}
  def self.must_because(arg, &reason_blk); end

  # A way to assert that a given branch of control flow is unreachable.
  #
  # Most commonly used to assert that a `case` or `if` expression exhaustively
  # handles all possible input types.
  #
  # For more information, see https://sorbet.org/docs/exhaustiveness
  sig {params(value: BasicObject).returns(T.noreturn)}
  def self.absurd(value); end
end

module T::Generic
  include T::Helpers

  # Type syntax for declaring a generic type variable scoped to instances of
  # the enclosing class. Most commonly used to create generic container data
  # types.
  #
  # For more information, see https://sorbet.org/docs/generics
  sig {params(variance: Symbol, blk: T.untyped).returns(T::Types::TypeMember)}
  def type_member(variance=:invariant, &blk); end

  # Type syntax for declaring a generic type variable scoped to the singleton
  # class of the enclosing class. Most commonly used to simulate having an
  # "abstract" type which is made concrete by a child class.
  #
  # For more information, see https://sorbet.org/docs/generics
  sig {params(variance: Symbol, blk: T.untyped).returns(T::Types::TypeTemplate)}
  def type_template(variance=:invariant, &blk); end

  # Type syntax for applying a generic class to a type argument.
  #
  # Note that Sorbet implements runtime-erased generics, which means that
  # generic classes and normal classes are indistinguishable at runtime.
  # Consequently, at runtime this method drops the type arguments on the floor
  # and simply evaluates to the receiver: `Box[Integer]` evaluates to simply
  # the `Box` class object.
  #
  # For more information, see https://sorbet.org/docs/generics#generics-and-runtime-checks
  def [](*types); end

  # Allows using `T.attached_class` in this module, at the expense of only
  # being allowed to `extend` this module, never `include` it (unless the
  # module it's included into is also marked `has_attached_class!`).
  #
  # For more information, see https://sorbet.org/docs/attached-class
  sig {params(variance: Symbol, blk: T.untyped).void}
  def has_attached_class!(variance=:invariant, &blk); end
end

module T::Helpers
  # Type syntax for declaring an abstract class or module.
  #
  # For more information, see https://sorbet.org/docs/abstract
  sig {void}
  def abstract!;  end

  # Type syntax for declaring an interface, which is an abstract module where
  # all the methods are abstract.
  #
  # For more information, see https://sorbet.org/docs/abstract
  sig {void}
  def interface!; end

  # Type syntax for declaring a final class or module, which prevents it from
  # being included, extended, or inherited.
  #
  # For more information, see https://sorbet.org/docs/final
  sig {void}
  def final!; end

  # Type syntax for declaring a sealed class or module, which limits the class
  # or module to being included, extended, or inherited except in the class
  # where the sealed module itself is defined. Most commonly used to declare
  # algebraic data types.
  #
  # For more information, see https://sorbet.org/docs/sealed
  sig {void}
  def sealed!; end

  # We do not use signatures on `mixes_in_class_methods` as to not duplicate errors
  # between the `namer` phase and typechecking for calls.
  #
  # With the following example:
  #
  # ```
  # class A
  #   mixes_in_class_methods A
  # end
  # ```
  #
  # Using a signature would mean we would generate two errors:
  # 1. error: `mixes_in_class_methods` must only contain constant literals (from namer)
  # 2. error: Expected `Module` but found `NilClass` for argument `mod` (from calls)

  # Used to allow a module which is mixed into a class with `include` to also
  # mix-in methods from the provided module onto the singleton class of the
  # `include` target.
  #
  # For more information, see https://sorbet.org/docs/abstract#interfaces-and-the-included-hook
  sig {params(mod: Module, mods: Module).void}
  def mixes_in_class_methods(mod, *mods); end

  # Experimental feature to require that a module be eventually mixed into a
  # given type of module.
  #
  # For more information, see https://sorbet.org/docs/requires-ancestor
  sig { params(block: T.proc.void).void }
  def requires_ancestor(&block); end
end

module T::Array
  # Type syntax to specify the element type of a standard library Array
  def self.[](type); end
end
module T::Hash
  # Type syntax to specify the key and value types of a standard library Hash
  def self.[](keys, values); end
end
module T::Set
  # Type syntax to specify the element type of a standard library Set
  def self.[](type); end
end
module T::Range
  # Type syntax to specify the element type of a standard library Range
  def self.[](type); end
end
module T::Class
  # Type syntax to specify the element type of a standard library Class
  def self.[](type); end
end
module T::Enumerable
  # Type syntax to specify the element type of a standard library Enumerable
  def self.[](type); end
end
module T::Enumerator
  # Type syntax to specify the element type of a standard library Enumerator
  def self.[](type); end
end
module T::Enumerator::Lazy
  # Type syntax to specify the element type of a standard library Enumerator::Lazy
  def self.[](type); end
end
module T::Enumerator::Chain
  # Type syntax to specify the element type of a standard library Enumerator::Chain
  def self.[](type); end
end

# Type syntax for either a `true` or `false` value.
#
# Note that `T::Boolean` is merely a shorthand, defined inside Sorbet as
#
# ```ruby
# T::Boolean = T.type_alias {T.any(TrueClass, FalseClass)}
# ```
#
# For more information, see https://sorbet.org/docs/class-types#booleans
T::Boolean = T.type_alias {T.any(TrueClass, FalseClass)}

module T::Configuration
  def self.call_validation_error_handler(signature, opts={}); end
  def self.call_validation_error_handler=(value); end
  def self.default_checked_level=(default_checked_level); end
  def self.enable_checking_for_sigs_marked_checked_tests; end
  def self.enable_final_checks_on_hooks; end
  def self.enable_legacy_t_enum_migration_mode; end
  def self.reset_final_checks_on_hooks; end
  sig {returns(T::Boolean)}
  def self.include_value_in_type_errors?; end
  sig {void}
  def self.exclude_value_in_type_errors; end
  sig {void}
  def self.include_value_in_type_errors; end
  def self.can_enable_vm_prop_serde?; end
  def self.use_vm_prop_serde?; end
  def self.enable_vm_prop_serde; end
  def self.disable_vm_prop_serde; end
  def self.hard_assert_handler(str, extra); end
  def self.hard_assert_handler=(value); end
  def self.inline_type_error_handler(error, opts={}); end
  def self.inline_type_error_handler=(value); end
  def self.log_info_handler(str, extra); end
  def self.log_info_handler=(value); end
  def self.module_name_mangler; end
  def self.module_name_mangler=(value); end
  def self.scalar_types; end
  def self.scalar_types=(values); end
  def self.sealed_violation_whitelist; end
  def self.sealed_violation_whitelist=(sealed_violation_whitelist); end
  def self.sig_builder_error_handler(error, location); end
  def self.sig_builder_error_handler=(value); end
  def self.sig_validation_error_handler(error, opts={}); end
  def self.sig_validation_error_handler=(value); end
  def self.soft_assert_handler(str, extra); end
  def self.soft_assert_handler=(value); end
  def self.normalize_sensitivity_and_pii_handler=(handler); end
  def self.normalize_sensitivity_and_pii_handler; end
  def self.redaction_handler=(handler); end
  def self.redaction_handler; end
  def self.class_owner_finder=(handler); end
  def self.class_owner_finder; end

  sig {params(handler: T.proc.params(arg0: T::Props::Decorator::DecoratedInstance, arg1: Symbol).void).void}
  def self.prop_freeze_handler=(handler); end
  sig {returns(T.proc.params(arg0: T::Props::Decorator::DecoratedInstance, arg1: Symbol).void)}
  def self.prop_freeze_handler; end
end

module T::Utils
  def self.arity(method); end

  # Converts Sorbet type syntax into a T::Types::Base instance to provide
  # access to run-time Sorbet type information.
  sig {params(val: T.untyped).returns(T::Types::Base)}
  def self.coerce(val); end

  def self.resolve_alias(type); end
  def self.run_all_sig_blocks; end
  def self.signature_for_method(method); end
  def self.signature_for_instance_method(mod, method_name); end
  def self.unwrap_nilable(type); end
  def self.wrap_method_with_call_validation_if_needed(mod, method_sig, original_method); end
  def self.check_type_recursive!(value, type); end

  # only one caller; delete
  def self.methods_excluding_object(mod); end
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
  sig {params(val: BasicObject, klass: String, package: T.nilable(String)).returns(T::Boolean)}
  def self.non_forcing_is_a?(val, klass, package: nil); end
end
