# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative './lib/sorbet-runtime'

module SorbetBenchmarks
  module Typecheck
    extend T::Sig

    sig { params(x: Integer).void }
    def self.integer_param(x)
    end

    def self.integer_param_module_eval(x)
      raise
    end

    module_eval(<<~RUBY)
      class << self
        alias_method :"integer_param_module_eval (original)", :integer_param_module_eval
        def integer_param_module_eval(arg0)
          unless arg0.is_a?(Integer)
            method_sig = T::Utils.signature_for_method(method(:integer_param))
            T::Private::Methods::CallValidation.report_error(
              method_sig,
              method_sig.arg_types[0][1].error_message_for_obj(arg0),
              'Parameter',
              method_sig.arg_types[0][0],
              Integer,
              arg0,
              caller_offset: 0
            )
          end

          send(:"integer_param_module_eval (original)", arg0)

          T::Private::Types::Void::VOID
        end
      end
    RUBY

    integer_param_module_eval(0)
  end
end
