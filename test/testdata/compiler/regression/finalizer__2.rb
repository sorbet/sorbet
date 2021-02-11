# frozen_string_literal: true
# typed: strict
# compiled: true

module Opus; end
module Opus::Utils
  module DeepFreeze
    extend T::Sig

    # Freeze a given object, and return all sub-items that also need freezing
    sig {params(todo: T::Array[T.untyped], obj: T.untyped).void.checked(:tests)}
    private_class_method def self.freeze_one(todo, obj)
      case obj
      when Module
        # Skip freezing modules/classes, they're very different
      when Array, Struct
        obj.freeze
        # You can't concat a struct, but it has each so you can keep array/struct handling the same
        obj.each do |value|
          todo << value
        end
      when Hash
        obj.freeze
        obj.each do |key, value|
          todo << key
          todo << value
        end
      when Range
        obj.freeze
        todo << obj.begin
        todo << obj.end
      else
        # Default to just freezing all instance variables
        obj.freeze
        obj.instance_variables.each do |iv|
          todo << obj.instance_variable_get(iv) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
        end
      end
    end

    sig do
      type_parameters(:T)
      .params(obj: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
      .checked(:tests)
    end
    def self.freeze_unchecked!(obj)
      todo = [T.unsafe(obj)]
      seen = {}

      until todo.empty?
        o = todo.pop

        case o
        when NilClass, TrueClass, FalseClass
          # don't need to be frozen
          nil
        # Short circuit on common classes.
        # Dispatch on one class, so that the compiler has separate inline caches per function call
        when Symbol
          o.freeze
        when Numeric
          o.freeze
        when String
          o.freeze
        else
          # Skip if we've already seen this object
          if !seen[o.object_id]
            seen[o.object_id] = true
            freeze_one(todo, o)
          end
        end
      end

      obj
    end

    # Freeze the given object, and everything it contains. Returns the original object.
    sig do
      type_parameters(:T)
      .params(obj: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
      .checked(:tests) # runtime typechecking disabled because of api/lib/test/critical_methods_no_runtime_typing.rb
    end
    def self.freeze!(obj)
      freeze_unchecked!(obj)
    end
  end
end
