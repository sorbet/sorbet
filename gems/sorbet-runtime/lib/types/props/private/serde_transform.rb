# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    module SerdeTransform
      extend T::Sig

      class Serialize; end
      private_constant :Serialize
      class Deserialize; end
      private_constant :Deserialize
      ModeType = T.type_alias {T.any(Serialize, Deserialize)}
      private_constant :ModeType

      module Mode
        SERIALIZE = T.let(Serialize.new.freeze, Serialize)
        DESERIALIZE = T.let(Deserialize.new.freeze, Deserialize)
      end

      NO_TRANSFORM_TYPES = T.let(
        [TrueClass, FalseClass, NilClass, Symbol, String, Numeric].freeze,
        T::Array[Module],
      )
      private_constant :NO_TRANSFORM_TYPES

      sig do
        params(
          type: T.any(T::Types::Base, Module),
          mode: ModeType,
          varname: String,
        )
        .returns(T.nilable(String))
        .checked(:never)
      end
      def self.generate(type, mode, varname)
        case type
        when T::Types::TypedArray
          inner = generate(type.type, mode, 'v')
          if inner.nil?
            "#{varname}.dup"
          else
            "#{varname}.map {|v| #{inner}}"
          end
        when T::Types::TypedSet
          inner = generate(type.type, mode, 'v')
          if inner.nil?
            "#{varname}.dup"
          else
            "Set.new(#{varname}) {|v| #{inner}}"
          end
        when T::Types::TypedHash
          keys = generate(type.keys, mode, 'k')
          values = generate(type.values, mode, 'v')
          if keys && values
            "#{varname}.each_with_object({}) {|(k,v),h| h[#{keys}] = #{values}}"
          elsif keys
            "#{varname}.transform_keys {|k| #{keys}}"
          elsif values
            "#{varname}.transform_values {|v| #{values}}"
          else
            "#{varname}.dup"
          end
        when T::Types::Simple
          raw = type.raw_type
          if NO_TRANSFORM_TYPES.any? {|cls| raw <= cls}
            nil
          elsif raw < T::Props::Serializable
            handle_serializable_subtype(varname, raw, mode)
          elsif raw.singleton_class < T::Props::CustomType
            handle_custom_type(varname, T.unsafe(raw), mode)
          elsif T::Configuration.scalar_types.include?(raw.name)
            # It's a bit of a hack that this is separate from NO_TRANSFORM_TYPES
            # and doesn't check inheritance (like `T::Props::CustomType.scalar_type?`
            # does), but it covers the main use case (pay-server's custom `Boolean`
            # module) without either requiring `T::Configuration.scalar_types` to
            # accept modules instead of strings (which produces load-order issues
            # and subtle behavior changes) or eating the performance cost of doing
            # an inheritance check by manually crawling a class hierarchy and doing
            # string comparisons.
            nil
          else
            "T::Props::Utils.deep_clone_object(#{varname})"
          end
        when T::Types::Union
          non_nil_type = T::Utils.unwrap_nilable(type)
          if non_nil_type
            inner = generate(non_nil_type, mode, varname)
            if inner.nil?
              nil
            else
              "#{varname}.nil? ? nil : #{inner}"
            end
          else
            # Handle, e.g., T::Boolean
            if type.types.all? {|t| generate(t, mode, varname).nil?}
              nil
            else
              "T::Props::Utils.deep_clone_object(#{varname})"
            end
          end
        when T::Types::Enum
          generate(T::Utils.lift_enum(type), mode, varname)
        else
          if type.singleton_class < T::Props::CustomType
            # Sometimes this comes wrapped in a T::Types::Simple and sometimes not
            handle_custom_type(varname, T.unsafe(type), mode)
          else
            "T::Props::Utils.deep_clone_object(#{varname})"
          end
        end
      end

      sig {params(varname: String, type: Module, mode: ModeType).returns(String).checked(:never)}
      private_class_method def self.handle_serializable_subtype(varname, type, mode)
        case mode
        when Serialize
          "#{varname}.serialize(strict)"
        when Deserialize
          type_name = T.must(module_name(type))
          "#{type_name}.from_hash(#{varname})"
        else
          T.absurd(mode)
        end
      end

      sig {params(varname: String, type: Module, mode: ModeType).returns(String).checked(:never)}
      private_class_method def self.handle_custom_type(varname, type, mode)
        case mode
        when Serialize
          type_name = T.must(module_name(type))
          "T::Props::CustomType.checked_serialize(#{type_name}, #{varname})"
        when Deserialize
          type_name = T.must(module_name(type))
          "#{type_name}.deserialize(#{varname})"
        else
          T.absurd(mode)
        end
      end

      # Guard against overrides of `name` or `to_s`
      MODULE_NAME = T.let(Module.instance_method(:name), UnboundMethod)
      private_constant :MODULE_NAME

      sig {params(type: Module).returns(T.nilable(String)).checked(:never)}
      private_class_method def self.module_name(type)
        MODULE_NAME.bind(type).call
      end
    end
  end
end
