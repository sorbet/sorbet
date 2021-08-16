# frozen_string_literal: true
# typed: true
# compiled: true

require 'benchmark'

#T::Configuration.enable_vm_prop_serde

module SorbetBenchmarks
  module Deserialize
    N_PROPS = 70
    ITERATIONS = 1_000_000

    # from sorbet/gems/sorbet-runtime/bench/deserialize.rb
    class Example
      include T::Props::Serializable

      class Subdoc
        include T::Props::Serializable

        prop :prop, String
      end

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

    # somewhat representative of expensive things inside Stripe
    class Model < T::Struct
    end

    N_PROPS.times do |i|
      Model.prop :"prop#{i}", T.nilable(String)
    end

    def self.run
      example_mostly_nil_input = {
        'prop3' => 0,
        'prop4' => [],
        'prop6' => {},
      }

      example_no_custom_types_input = {
        'prop1' => 0,
        'prop2' => 0,
        'prop3' => 0,
        'prop4' => [1, 2, 3],
        'prop5' => [1, 2, 3],
        'prop6' => {'foo' => 1, 'bar' => 2},
        'prop7' => {'foo' => 1, 'bar' => 2},
      }

      example_complete_input = {
        'prop1' => 0,
        'prop2' => 0,
        'prop3' => 0,
        'prop4' => [1, 2, 3],
        'prop5' => [1, 2, 3],
        'prop6' => {'foo' => 1, 'bar' => 2},
        'prop7' => {'foo' => 1, 'bar' => 2},
        'prop8' => {'prop' => ''},
        'prop9' => [{'prop' => ''}, {'prop' => ''}],
        'prop10' => {'foo' => {'prop' => ''}, 'bar' => {'prop' => ''}},
      }

      model_nil_input = {}
      model_half_present_input = (1..(N_PROPS/2)).each_with_object({}) do |i, obj|
        obj["prop#{i}"] = "#{i}"
      end
      model_all_present_input = (1..N_PROPS).each_with_object({}) do |i, obj|
        obj["prop#{i}"] = "#{i}"
      end

      e = Example.allocate
      m = Model.allocate

      GC.disable

      Benchmark.bmbm do |x|
        x.report('sorbet-runtime Example, mostly nil input') do
          ITERATIONS.times do
            e.deserialize(example_mostly_nil_input, false)
          end
        end

        x.report('sorbet-runtime Example, no custom types input') do
          ITERATIONS.times do
            e.deserialize(example_no_custom_types_input, false)
          end
        end

        x.report('sorbet-runtime Example, all props set') do
          ITERATIONS.times do
            e.deserialize(example_complete_input, false)
          end
        end

        x.report('Model, all nil input') do
          ITERATIONS.times do
            m.deserialize(model_nil_input, false)
          end
        end

        x.report('Model, half present input') do
          ITERATIONS.times do
            m.deserialize(model_half_present_input, false)
          end
        end

        x.report('Model, all present input') do
          ITERATIONS.times do
            m.deserialize(model_all_present_input, false)
          end
        end
      end
    end
  end
end

SorbetBenchmarks::Deserialize.run
