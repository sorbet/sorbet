# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    module SetterFactory
      extend T::Sig

      SetterProc = T.type_alias {T.proc.params(val: T.untyped).void}
      ValidateProc = T.type_alias {T.proc.params(prop: Symbol, value: T.untyped).void}

      sig do
        params(
          klass: T.all(Module, T::Props::ClassMethods),
          prop: Symbol,
          rules: T::Hash[Symbol, T.untyped]
        )
        .returns(SetterProc)
        .checked(:never)
      end
      def self.build_setter_proc(klass, prop, rules)
        # Our nil check works differently than a simple T.nilable for various
        # reasons (including the `raise_on_nil_write` setting, the existence
        # of defaults & factories, and the fact that we allow `T.nilable(Foo)`
        # where Foo < T::Props::CustomType as a prop type even though calling
        # `valid?` on it won't work as expected), so unwrap any T.nilable and
        # do a check manually. (Note this hack does not fix custom types as
        # collection elements.)
        non_nil_type = T::Utils::Nilable.get_underlying_type_object(rules.fetch(:type_object))
        if non_nil_type.is_a?(T::Types::Simple) && non_nil_type.raw_type.singleton_class < T::Props::CustomType
          non_nil_type = non_nil_type.raw_type
        end

        accessor_key = rules.fetch(:accessor_key)
        validate = rules[:setter_validate]

        # It seems like a bug that this affects the behavior of setters, but
        # some existing code relies on this behavior
        has_explicit_nil_default = rules.key?(:default) && rules.fetch(:default).nil?

        # Use separate methods in order to ensure that we only close over necessary
        # variables
        if !T::Props::Utils.need_nil_write_check?(rules) || has_explicit_nil_default
          nilable_proc(prop, accessor_key, non_nil_type, klass, validate)
        else
          non_nil_proc(prop, accessor_key, non_nil_type, klass, validate)
        end
      end

      sig do
        params(
          prop: Symbol,
          accessor_key: Symbol,
          non_nil_type: T.any(T::Types::Base, T.all(T::Props::CustomType, Module)),
          klass: T.all(Module, T::Props::ClassMethods),
          validate: T.nilable(ValidateProc)
        )
        .returns(SetterProc)
      end
      private_class_method def self.non_nil_proc(prop, accessor_key, non_nil_type, klass, validate)
        proc do |val|
          if non_nil_type.valid?(val)
            if validate
              validate.call(prop, val)
            end
            instance_variable_set(accessor_key, val)
          else
            T::Props::Private::SetterFactory.raise_pretty_error(
              klass,
              prop,
              non_nil_type,
              val,
            )
          end
        end
      end

      sig do
        params(
          prop: Symbol,
          accessor_key: Symbol,
          non_nil_type: T.any(T::Types::Base, T.all(T::Props::CustomType, Module)),
          klass: T.all(Module, T::Props::ClassMethods),
          validate: T.nilable(ValidateProc),
        )
        .returns(SetterProc)
      end
      private_class_method def self.nilable_proc(prop, accessor_key, non_nil_type, klass, validate)
        proc do |val|
          if val.nil?
            instance_variable_set(accessor_key, nil)
          elsif non_nil_type.valid?(val)
            if validate
              validate.call(prop, val)
            end
            instance_variable_set(accessor_key, val)
          else
            T::Props::Private::SetterFactory.raise_pretty_error(
              klass,
              prop,
              non_nil_type,
              val,
            )
          end
        end
      end

      sig do
        params(
          klass: T.all(Module, T::Props::ClassMethods),
          prop: Symbol,
          type: T.any(T::Types::Base, Module),
          val: T.untyped,
        )
        .void
      end
      def self.raise_pretty_error(klass, prop, type, val)
        base_message = "Can't set #{klass.name}.#{prop} to #{val.inspect} (instance of #{val.class}) - need a #{type}"

        pretty_message = "Parameter '#{prop}': #{base_message}\n"
        caller_loc = caller_locations&.find {|l| !l.to_s.include?('sorbet-runtime/lib/types/props')}
        if caller_loc
          pretty_message += "Caller: #{caller_loc.path}:#{caller_loc.lineno}\n"
        end

        T::Configuration.call_validation_error_handler(
          nil,
          message: base_message,
          pretty_message: pretty_message,
          kind: 'Parameter',
          name: prop,
          type: type,
          value: val,
          location: caller_loc,
        )
      end
    end
  end
end
