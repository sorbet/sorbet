# frozen_string_literal: true
# typed: true
# compiled: true

require 'benchmark'

class Module
  include T::Sig
end

module Opus; end
module Opus::Utils
  module DeepFreeze
    # Freeze a given object, and return all sub-items that also need freezing
    sig(:final) {params(todo: T::Array[T.untyped], obj: T.untyped).void.checked(:tests)}
    def self.freeze_one(todo, obj)
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

    sig(:final) do
      type_parameters(:T)
      .params(obj: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
      .checked(:never)
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
        # Dispatch on one class, so that the compiler has more static information per function call
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
    sig(:final) do
      type_parameters(:T)
      .params(obj: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
      .checked(:never) # runtime typechecking disabled because of test/functional/api/critical_methods_no_runtime_typing.rb
    end
    def self.freeze!(obj)
      freeze_unchecked!(obj)
    end
  end
end

module Opus::Utils::DeepFreeze
  class SomeClass
    sig {params(foo: T.untyped).void}
    def initialize(foo)
      @foo = foo
    end
  end

  sig {params(n: Integer).returns(T::Array[T.untyped])}
  def self.generate_deep(n)
    o = T.let("foo".dup, T.untyped)
    n.times do
      o = SomeClass.new(o)
      o = [o, "bar".dup]
      o = {a: o, b: "iggy".dup}
      o = Struct.new(:o, :p).new(o, "p".dup)
      o = [1, 2, 3, o, 4]
    end
    o
  end

  GENERATORS = {
    nil: [5e5, -> {nil}],
    string: [5e5, -> {"foo".dup}],
    array: [1e4, -> {Array.new(100) {"foo".dup}}],
    recursive: [200, -> {Opus::Utils::DeepFreeze.generate_deep(1)}],
    deep: [200, -> {Opus::Utils::DeepFreeze.generate_deep(30)}],
  }

  def self.main
    GENERATORS.each do |name, (iterations, g)|
      objs = 1.upto(iterations).map {g[]}
      objs.each {|o| Opus::Utils::DeepFreeze.freeze!(o)}
    end

    0
  end
end

Opus::Utils::DeepFreeze.main
