# typed: true
# frozen_string_literal: true

module T::Private::ErrorHandler
  # Handle a rescued TypeError. This will pass the TypeError to the
  # T::Configuration.inline_type_error_handler so that the user can override
  # error handling if they wish. If no inline_type_error_handler is set, this
  # method will call handle_inline_type_error_default, which raises the error.
  def self.handle_inline_type_error(type_error)
    T::Configuration.inline_type_error_handler(type_error)
    nil
  end

  # Handle a sig declaration failure. This allows users to override the behavior
  # when a sig decl fails. If T::Configuration.sig_builder_error_handler
  # is unset, this method will call handle_sig_builder_error_default.
  def self.handle_sig_builder_error(error, location)
    T::Configuration.sig_builder_error_handler(error, location)
    nil
  end

  # Handle a sig build validation failure. This allows users to override the
  # behavior when a sig build fails. If T::Configuration.sig_validation_error_handler
  # is unset, this method will call handle_sig_validation_error_default.
  def self.handle_sig_validation_error(error, opts={})
    T::Configuration.sig_validation_error_handler(error, opts)
    nil
  end

  # Handle a sig call validation failure. This allows users to override the
  # behavior when a sig call fails. If T::Configuration.call_validation_error_handler
  # is unset, this method will call handle_call_validation_error_default.
  def self.handle_call_validation_error(signature, opts={})
    T::Configuration.call_validation_error_handler(signature, opts)
    nil
  end
end
