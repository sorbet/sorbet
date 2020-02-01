# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    module SerdeTransform
      extend T::Sig

      TrustedRuby = RubyGen::TrustedRuby

      class Mode < T::Enum
        enums do
          SERIALIZE = new
          DESERIALIZE = new
        end
      end

      class HasVarname < RubyGen::Template
        abstract!

        sig {params(name: TrustedRuby).returns(TrustedRuby).checked(:never)}
        def self.generate(name)
          new(name: name).generate
        end
      end

      class HasVarnameAndInner < RubyGen::Template
        abstract!

        sig {params(name: TrustedRuby, inner: TrustedRuby).returns(TrustedRuby).checked(:never)}
        def self.generate(name, inner)
          new(name: name, inner: inner).generate
        end
      end

      class HasVarnameAndSubtype < RubyGen::Template
        abstract!

        sig {params(name: TrustedRuby, subtype: Module).returns(TrustedRuby).checked(:never)}
        def self.generate(name, subtype)
          new(name: name, subtype: RubyGen::ModuleLiteral.new(subtype)).generate
        end
      end

      class None < HasVarname
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s'
        end
      end

      class Dup < HasVarname
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.dup'
        end
      end

      class SerializeProps < HasVarname
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.serialize(strict)'
        end
      end

      class DeserializeProps < HasVarnameAndSubtype
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<subtype>s.from_hash(%<name>s)'
        end
      end

      class SerializeCustomType < HasVarnameAndSubtype
        sig {override.returns(String).checked(:never)}
        def self.format_string
          'T::Props::CustomType.checked_serialize(%<subtype>s, %<name>s)'
        end
      end

      class DeserializeCustomType < HasVarnameAndSubtype
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<subtype>s.deserialize(%<name>s)'
        end
      end

      class DynamicDeepClone < HasVarname
        sig {override.returns(String).checked(:never)}
        def self.format_string
          'T::Props::Utils.deep_clone_object(%<name>s)'
        end
      end

      class Map < HasVarnameAndInner
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.map {|v| %<inner>s}'
        end
      end

      class MapSet < HasVarnameAndInner
        sig {override.returns(String).checked(:never)}
        def self.format_string
          'Set.new(%<name>s) {|v| %<inner>s}'
        end
      end

      class TransformValues < HasVarnameAndInner
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.transform_values {|v| %<inner>s}'
        end
      end

      class TransformKeys < HasVarnameAndInner
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.transform_keys {|k| %<inner>s}'
        end
      end

      class TransformKeyValues < RubyGen::Template
        sig {params(name: TrustedRuby, keys: TrustedRuby, values: TrustedRuby).returns(TrustedRuby).checked(:never)}
        def self.generate(name, keys:, values:)
          new(name: name, keys: keys, values: values).generate
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.each_with_object({}) {|(k,v),h| h[%<keys>s] = %<values>s}'
        end
      end

      class IfNotNil < HasVarnameAndInner
        sig {override.returns(String).checked(:never)}
        def self.format_string
          '%<name>s.nil? ? nil : %<inner>s'
        end
      end

      INPUT_VARNAME = T.let(TrustedRuby.constant('val'), TrustedRuby)
      KEY_VARNAME = T.let(TrustedRuby.constant('k'), TrustedRuby)
      VALUE_VARNAME = T.let(TrustedRuby.constant('v'), TrustedRuby)
      NO_TRANSFORM_TYPES = T.let([TrueClass, FalseClass, NilClass, Symbol, String, Numeric].freeze, T::Array[Module])
      private_constant :INPUT_VARNAME, :KEY_VARNAME, :VALUE_VARNAME, :NO_TRANSFORM_TYPES

      sig do
        params(
          type: T.any(T::Types::Base, Module),
          mode: Mode,
        )
        .returns(TrustedRuby)
        .checked(:never)
      end
      def self.generate(type, mode)
        for_type_and_var(type, mode, INPUT_VARNAME) || INPUT_VARNAME
      end

      sig do
        params(
          type: T.any(T::Types::Base, Module),
          mode: Mode,
          varname: TrustedRuby,
        )
        .returns(T.nilable(TrustedRuby))
        .checked(:never)
      end
      private_class_method def self.for_type_and_var(type, mode, varname)
        case type
        when T::Types::TypedArray
          inner = for_type_and_var(type.type, mode, VALUE_VARNAME)
          if inner.nil?
            Dup.generate(varname)
          else
            Map.generate(varname, inner)
          end
        when T::Types::TypedSet
          inner = for_type_and_var(type.type, mode, VALUE_VARNAME)
          if inner.nil?
            Dup.generate(varname)
          else
            MapSet.generate(varname, inner)
          end
        when T::Types::TypedHash
          keys = for_type_and_var(type.keys, mode, KEY_VARNAME)
          values = for_type_and_var(type.values, mode, VALUE_VARNAME)
          if keys && values
            TransformKeyValues.generate(varname, keys: keys, values: values)
          elsif keys
            TransformKeys.generate(varname, keys)
          elsif values
            TransformValues.generate(varname, values)
          else
            Dup.generate(varname)
          end
        when T::Types::Simple
          raw = type.raw_type
          if NO_TRANSFORM_TYPES.any? {|cls| raw <= cls}
            nil
          elsif raw < T::Props::Serializable
            case mode
            when Mode::SERIALIZE
              SerializeProps.generate(varname)
            when Mode::DESERIALIZE
              DeserializeProps.generate(varname, raw)
            else
              T.absurd(mode)
            end
          elsif raw.singleton_class < T::Props::CustomType
            handle_custom_type(varname, T.unsafe(raw), mode)
          else
            DynamicDeepClone.generate(varname)
          end
        when T::Types::Union
          non_nil_type = T::Utils.unwrap_nilable(type)
          if non_nil_type
            inner = for_type_and_var(non_nil_type, mode, varname)
            if inner.nil?
              nil
            else
              IfNotNil.generate(varname, inner)
            end
          else
            DynamicDeepClone.generate(varname)
          end
        else
          if type.singleton_class < T::Props::CustomType
            # Sometimes this comes wrapped in a T::Types::Simple and sometimes not
            handle_custom_type(varname, T.unsafe(type), mode)
          else
            DynamicDeepClone.generate(varname)
          end
        end
      end

      sig {params(varname: TrustedRuby, type: Module, mode: Mode).returns(TrustedRuby).checked(:never)}
      private_class_method def self.handle_custom_type(varname, type, mode)
        case mode
        when Mode::SERIALIZE
          SerializeCustomType.generate(varname, type)
        when Mode::DESERIALIZE
          DeserializeCustomType.generate(varname, type)
        else
          T.absurd(mode)
        end
      end
    end
  end
end
