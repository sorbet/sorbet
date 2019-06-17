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
    validate_lambda_given!(value)
    @inline_type_error_handler = value
  end

  private_class_method def self.inline_type_error_handler_default(error)
    raise error
  end

  def self.inline_type_error_handler(error)
    if @inline_type_error_handler
      @inline_type_error_handler.call(error)
    else
      inline_type_error_handler_default(error)
    end
  end

  # Set a handler to handle errors that occur when the builder methods in the
  # body of a sig are executed. The sig builder methods are inside a proc so
  # that they can be lazily evaluated the first time the method being sig'd is
  # called.
  #
  # By default, improper use of the builder methods within the body of a sig
  # cause an ArgumentError to be raised. Setting sig_builder_error_handler to an
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
  #   T::Configuration.sig_builder_error_handler = lambda do |error, location|
  #     puts error.message
  #   end
  def self.sig_builder_error_handler=(value)
    validate_lambda_given!(value)
    @sig_builder_error_handler = value
  end

  private_class_method def self.sig_builder_error_handler_default(error, location)
    T::Private::Methods.sig_error(location, error.message)
  end

  def self.sig_builder_error_handler(error, location)
    if @sig_builder_error_handler
      @sig_builder_error_handler.call(error, location)
    else
      sig_builder_error_handler_default(error, location)
    end
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
    validate_lambda_given!(value)
    @sig_validation_error_handler = value
  end

  private_class_method def self.sig_validation_error_handler_default(error, opts)
    # if this method overrides a generated signature, report that one instead
    bad_method = opts[:method]
    if !opts[:declaration].generated
      super_signature = opts[:super_signature]
      raise error if !super_signature&.generated
      bad_method = super_signature.method
    end

    method_file, method_line = bad_method.source_location
    T::Configuration.log_info_handler(
      "SIG-DECLARE-FAILED",
      {
        definition_file: method_file,
        definition_line: method_line,
        kind: "Delete",
        message: error.message,
      },
    )
  end

  def self.sig_validation_error_handler(error, opts)
    if @sig_validation_error_handler
      @sig_validation_error_handler.call(error, opts)
    else
      sig_validation_error_handler_default(error, opts)
    end
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
    validate_lambda_given!(value)
    @call_validation_error_handler = value
  end

  private_class_method def self.call_validation_error_handler_default(signature, opts)
    method_file, method_line = signature.method.source_location
    location = opts[:location]
    suffix = "Caller: #{location.path}:#{location.lineno}\n" \
      "Definition: #{method_file}:#{method_line}"

    error_message = "#{opts[:kind]}#{opts[:name] ? " '#{opts[:name]}'" : ''}: #{opts[:message]}\n#{suffix}"

    if signature.generated
      got = opts[:value].class
      got = T.unsafe(T::Enumerable[T.untyped]).describe_obj(opts[:value]) if got < Enumerable
      T::Configuration.log_info_handler(
        "SIG-CHECK-FAILED",
        {
          caller_file: location.path,
          caller_line: location.lineno,
          definition_file: method_file,
          definition_line: method_line,
          kind: opts[:kind],
          name: opts[:name],
          expected: opts[:type].name,
          got: got,
        },
      )
    elsif signature.soft_notify
      T::Configuration.soft_assert_handler(
        "TypeError: #{error_message}",
        {notify: signature.soft_notify}
      )
    else
      begin
        raise TypeError.new(error_message)
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Private::ErrorHandler.handle_inline_type_error(e)
      end
    end
  end

  def self.call_validation_error_handler(signature, opts)
    if @call_validation_error_handler
      @call_validation_error_handler.call(signature, opts)
    else
      call_validation_error_handler_default(signature, opts)
    end
  end

  # Set a handler for logging
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error
  #   report (pass nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [String] str Message to be logged
  # @param [Hash] extra A hash containing additional parameters to be passed along to the logger.
  #
  # @example
  #   T::Configuration.log_info_handler = lambda do |str, extra|
  #     puts "#{str}, context: #{extra}"
  #   end
  def self.log_info_handler=(value)
    validate_lambda_given!(value)
    @log_info_handler = value
  end

  private_class_method def self.log_info_handler_default(str, extra)
    puts "#{str}, extra: #{extra}" # rubocop:disable PrisonGuard/NoBarePuts
  end

  def self.log_info_handler(str, extra)
    if @log_info_handler
      @log_info_handler.call(str, extra)
    else
      log_info_handler_default(str, extra)
    end
  end

  # Set a handler for soft assertions
  #
  # These generally shouldn't stop execution of the program, but rather inform
  # some party of the assertion to action on later.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error
  #   report (pass nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [String] str Assertion message
  # @param [Hash] extra A hash containing additional parameters to be passed along to the handler.
  #
  # @example
  #   T::Configuration.soft_assert_handler = lambda do |str, extra|
  #     puts "#{str}, context: #{extra}"
  #   end
  def self.soft_assert_handler=(value)
    validate_lambda_given!(value)
    @soft_assert_handler = value
  end

  private_class_method def self.soft_assert_handler_default(str, extra)
    puts "#{str}, extra: #{extra}" # rubocop:disable PrisonGuard/NoBarePuts
  end

  def self.soft_assert_handler(str, extra)
    if @soft_assert_handler
      @soft_assert_handler.call(str, extra)
    else
      soft_assert_handler_default(str, extra)
    end
  end

  # Set a handler for hard assertions
  #
  # These generally should stop execution of the program, and optionally inform
  # some party of the assertion.
  #
  # @param [Lambda, Proc, Object, nil] value Proc that handles the error
  #   report (pass nil to reset to default behavior)
  #
  # Parameters passed to value.call:
  #
  # @param [String] str Assertion message
  # @param [Hash] extra A hash containing additional parameters to be passed along to the handler.
  #
  # @example
  #   T::Configuration.hard_assert_handler = lambda do |str, extra|
  #     raise "#{str}, context: #{extra}"
  #   end
  def self.hard_assert_handler=(value)
    validate_lambda_given!(value)
    @hard_assert_handler = value
  end

  private_class_method def self.hard_assert_handler_default(str, _)
    raise str
  end

  def self.hard_assert_handler(str, extra)
    if @hard_assert_handler
      @hard_assert_handler.call(str, extra)
    else
      hard_assert_handler_default(str, extra)
    end
  end

  # Set a list of class strings that are to be considered scalar.
  #   (pass nil to reset to default behavior)
  #
  # @param [String] value Class name.
  #
  # @example
  #   T::Configuration.scalar_types = ["NilClass", "TrueClass", "FalseClass", ...]
  def self.scalar_types=(values)
    if values.nil?
      @scalar_tyeps = values
    else
      bad_values = values.select {|v| v.class != String}
      unless bad_values.empty?
        raise ArgumentError.new("Provided values must all be class name strings.")
      end

      @scalar_types = Set.new(values).freeze
    end
  end

  @default_scalar_types = Set.new(%w{
    NilClass
    TrueClass
    FalseClass
    Integer
    Float
    String
    Symbol
    Time
  }).freeze

  def self.scalar_types
    @scalar_types || @default_scalar_types
  end


  private_class_method def self.validate_lambda_given!(value)
    if !value.nil? && !value.respond_to?(:call)
      raise ArgumentError.new("Provided value must respond to :call")
    end
  end
end
