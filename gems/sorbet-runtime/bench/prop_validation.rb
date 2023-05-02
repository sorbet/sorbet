# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module PropValidation
    class Subdoc < T::Struct
      include T::Props::TypeValidation
      prop :prop, String
    end

    def self.run
      GC.disable
      before = GC.stat(:total_allocated_objects)
      result = Benchmark.measure do
        5_000.times do
          cls = Class.new(T::Struct) do
            include T::Props::TypeValidation
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
      after = GC.stat(:total_allocated_objects)

      puts "Subclassing T::Struct, with ten props (ms/iter):"
      puts result
      puts "Allocations: #{after - before}"
    end
  end
end
