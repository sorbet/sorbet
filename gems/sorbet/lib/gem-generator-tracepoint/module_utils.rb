# frozen_string_literal: true
# typed: true

module Sorbet::Private
  module GemGeneratorTracepoint
    class ModuleUtils
      @real_is_a = Module.instance_method(:is_a?)
      @real_name = Module.instance_method(:name)
      @real_object_id = Module.instance_method(:object_id)

      def self.real_is_a?(klass, type)
        @real_is_a.bind(klass).call(type)
      end

      def self.real_name(klass)
        @real_name.bind(klass).call
      end

      def self.real_object_id(klass)
        @real_object_id.bind(klass).call
      end
    end
  end
end
