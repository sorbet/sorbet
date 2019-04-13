# typed: true
# frozen_string_literal: true

module T::Configuration
  # Set a handler to handle `TypeError`s raised by the runtime type system.
  #
  # By default, any `TypeError`s detected by this gem will be raised. Setting
  # type_error_handler to an object that implements :call (e.g. proc or
  # lambda) allows users to customize the behavior when a `TypeError` is
  # raised.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error (pass
  #   nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [TypeError] error TypeError that was raised
  #
  # @example
  #   T::Configuration.type_error_handler = lambda do |error|
  #     puts error.message
  #   end
  def self.type_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @type_error_handler = value
  end

  def self.type_error_handler
    @type_error_handler
  end

  # Set a handler to handle sig declaration errors.
  #
  # By default, sig declaration errors cause an ArgumentError to be raised.
  # Setting sig_decl_error_handler to an object that implements :call (e.g. proc
  # or lambda) allows users to customize the behavior when a method signature
  # declaration is invalid.
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

  # Set a handler to handle sig build errors.
  #
  # By default, sig build errors cause either an Opus::Log message or a
  # StandardError (or subclass) to be raised, depending on the sig's
  # configuration (e.g. generated). Setting sig_build_error_handler to an
  # object that implements :call (e.g. proc or lambda) allows users to
  # customize the behavior when a method signature's build fails.
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
  #   T::Configuration.sig_build_error_handler = lambda do |error, opts|
  #     puts error.message
  #   end
  def self.sig_build_error_handler=(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
    @sig_build_error_handler = value
  end

  def self.sig_build_error_handler
    @sig_build_error_handler
  end

  # Set a handler to handle sig error reports.
  #
  # By default, sig errors cause either an Opus::Log message or a TypeError
  # to be raised, depending on the sig's configuration (e.g. generated or
  # soft). Setting call_validation_error_handler to an object that implements
  # :call (e.g. proc or lambda) allows users to customize the behavior when a
  # method is called with invalid parameters, or returns an invalid value.
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
