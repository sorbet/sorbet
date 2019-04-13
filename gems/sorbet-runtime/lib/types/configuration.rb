# typed: true
# frozen_string_literal: true

module T::Configuration
  # Set a handler to handle `TypeError`s raised by any in-line type assertions,
  # including `T.must`, `T.let`, `T.cast`, and `T.assert_type!`.
  #
  # By default, any `TypeError`s detected by this gem will be raised. Setting
  # inline_type_error_handler to an object that implements :call (e.g. proc or
  # lambda) allows users to customize the behavior when a `TypeError` is
  # raised on any inline type assertion.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error (pass
  #   nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [TypeError] error TypeError that was raised
  #
  # @example
  #   T::Configuration.inline_type_error_handler = lambda do |error|
  #     puts error.message
  #   end
  def self.inline_type_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @inline_type_error_handler = value
  end

  def self.inline_type_error_handler
    @inline_type_error_handler
  end

  # Set a handler to handle errors that occur when the builder methods in the
  # body of a sig are executed. The sig builder methods are inside a proc so
  # that they can be lazily evaluated the first time the method being sig'd is
  # called.
  #
  # By default, improper use of the builder methods within the body of a sig
  # cause an ArgumentError to be raised. Setting sig_decl_error_handler to an
  # object that implements :call (e.g. proc or lambda) allows users to
  # customize the behavior when a sig can't be built for some reason.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error (pass
  #   nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [StandardError] error The error that was raised
  # @param [Thread::Backtrace::Location] location Location of the error
  #
  # @example
  #   T::Configuration.sig_decl_error_handler = lambda do |error, location|
  #     puts error.message
  #   end
  def self.sig_decl_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @sig_decl_error_handler = value
  end

  def self.sig_decl_error_handler
    @sig_decl_error_handler
  end

  # Set a handler to handle sig validation errors.
  #
  # Sig validation errors include things like abstract checks, override checks,
  # and type compatibility of arguments. They happen after a sig has been
  # successfully built, but the built sig is incompatible with other sigs in
  # some way.
  #
  # By default, sig validation errors cause an exception to be raised. One
  # exception is for `generated` sigs, for which a message will be logged
  # instead of raising. Setting sig_validation_error_handler to an object that
  # implements :call (e.g. proc or lambda) allows users to customize the
  # behavior when a method signature's build fails.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error (pass
  #   nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [StandardError] error The error that was raised
  # @param [Hash] opts A hash containing contextual information on the error:
  # @option opts [Method, UnboundMethod] :method Method on which the signature build failed
  # @option opts [T::Private::Methods::Declaration] :declaration Method
  #   signature declaration struct
  # @option opts [T::Private::Methods::Signature, nil] :signature Signature
  #   that failed (nil if sig build failed before Signature initialization)
  # @option opts [T::Private::Methods::Signature, nil] :super_signature Super
  #   method's signature (nil if method is not an override or super method
  #   does not have a method signature)
  #
  # @example
  #   T::Configuration.sig_validation_error_handler = lambda do |error, opts|
  #     puts error.message
  #   end
  def self.sig_validation_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @sig_validation_error_handler = value
  end

  def self.sig_validation_error_handler
    @sig_validation_error_handler
  end

  # Set a handler for type errors that result from calling a method.
  #
  # By default, errors from calling a method cause an exception to be raised.
  # One exception is for `generated` sigs, for which a message will be logged
  # instead of raising. Setting call_validation_error_handler to an object that
  # implements :call (e.g. proc or lambda) allows users to customize the
  # behavior when a method is called with invalid parameters, or returns an
  # invalid value.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error
  #   report (pass nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [T::Private::Methods::Signature] signature Signature that failed
  # @param [Hash] opts A hash containing contextual information on the error:
  # @option opts [String] :message Error message
  # @option opts [String] :kind One of:
  #   ['Parameter', 'Block parameter', 'Return value']
  # @option opts [Symbol] :name Param or block param name (nil for return
  #   value)
  # @option opts [Object] :type Expected param/return value type
  # @option opts [Object] :value Actual param/return value
  # @option opts [Thread::Backtrace::Location] :location Location of the
  #   caller
  #
  # @example
  #   T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  #     puts opts[:message]
  #   end
  def self.call_validation_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @call_validation_error_handler = value
  end

  def self.call_validation_error_handler
    @call_validation_error_handler
  end
end
