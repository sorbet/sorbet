# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    module SerdeTransform
      extend T::Sig

      class Mode < T::Enum
        enums do
          SERIALIZE = new
          DESERIALIZE = new
        end
      end

      NO_TRANSFORM_TYPES = T.let(
        [TrueClass, FalseClass, NilClass, Symbol, String, Numeric].freeze,
        T::Array[Module],
      )
      private_constant :NO_TRANSFORM_TYPES

      sig do
        params(
          type: T.any(T::Types::Base, Module),
          mode: Mode,
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
            case mode
            when Mode::SERIALIZE
              "#{varname}.serialize(strict)"
            when Mode::DESERIALIZE
              "#{raw}.from_hash(#{varname})"
            else
              T.absurd(mode)
            end
          elsif raw.singleton_class < T::Props::CustomType
            handle_custom_type(varname, T.unsafe(raw), mode)
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
            "T::Props::Utils.deep_clone_object(#{varname})"
          end
        else
          if type.singleton_class < T::Props::CustomType
            # Sometimes this comes wrapped in a T::Types::Simple and sometimes not
            handle_custom_type(varname, T.unsafe(type), mode)
          else
            "T::Props::Utils.deep_clone_object(#{varname})"
          end
        end
      end

      sig {params(varname: String, type: Module, mode: Mode).returns(String).checked(:never)}
      private_class_method def self.handle_custom_type(varname, type, mode)
        case mode
        when Mode::SERIALIZE
          "T::Props::CustomType.checked_serialize(#{type}, #{varname})"
        when Mode::DESERIALIZE
          "#{type}.deserialize(#{varname})"
        else
          T.absurd(mode)
        end
      end
    end
  end
end
