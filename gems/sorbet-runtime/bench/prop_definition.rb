# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module PropDefinition

    class Subdoc < T::Struct
      prop :prop, String
    end

    def self.run
      100.times do
        cls = Class.new(T::Struct) do
          prop :prop, String
        end
        cls.decorator.eagerly_define_lazy_methods!
      end

      result = Benchmark.measure do
        1_000.times do
          cls = Class.new(T::Struct) do
            prop :prop, String
          end
          cls.decorator.eagerly_define_lazy_methods!
        end
      end

      puts "Subclassing T::Struct, with one prop (ms/iter):"
      puts result

      100.times do
        cls = Class.new(T::Struct) do
          prop :prop1, T.nilable(Integer)
          prop :prop2, Integer, default: 0
          prop :prop3, Integer
          prop :prop4, T::Array[Integer]
          prop :prop5, T::Array[Integer], default: []
          prop :prop6, T::Hash[String, Integer]
          prop :prop7, T::Hash[String, Integer], default: {}
          prop :prop8, T.nilable(Subdoc)
          prop :prop9, T::Array[Subdoc], default: []
          prop :prop10, T::Hash[String, Subdoc], default: {}
        end
        cls.decorator.eagerly_define_lazy_methods!
      end

      result = Benchmark.measure do
        1_000.times do
          cls = Class.new(T::Struct) do
            prop :prop1, T.nilable(Integer)
            prop :prop2, Integer, default: 0
            prop :prop3, Integer
            prop :prop4, T::Array[Integer]
            prop :prop5, T::Array[Integer], default: []
            prop :prop6, T::Hash[String, Integer]
            prop :prop7, T::Hash[String, Integer], default: {}
            prop :prop8, T.nilable(Subdoc)
            prop :prop9, T::Array[Subdoc], default: []
            prop :prop10, T::Hash[String, Subdoc], default: {}
          end
          cls.decorator.eagerly_define_lazy_methods!
        end
      end

      puts "Subclassing T::Struct, with ten props (ms/iter):"
      puts result
    end
  end
end

