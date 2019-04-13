# typed: true
# frozen_string_literal: true

module T::Private::ErrorHandler
  # Handle a rescued TypeError. This will pass the TypeError to the
  # T::Configuration.type_error_handler so that the user can override error
  # handling if they wish. If no type_error_handler is set, this method will
  # call handle_type_error_default, which raises the error.
  def self.handle_type_error(type_error)
    if T::Configuration.type_error_handler
      T::Configuration.type_error_handler.call(type_error)
    else
      handle_type_error_default(type_error)
    end
    nil
  end

  # Handle a sig declaration failure. This allows users to override the behavior
  # when a sig decl fails. If T::Configuration.sig_decl_error_handler
  # is unset, this method will call handle_sig_decl_error_default.
  def self.handle_sig_decl_error(error, location)
    if T::Configuration.sig_decl_error_handler
      T::Configuration.sig_decl_error_handler.call(error, location)
    else
      handle_sig_decl_error_default(error, location)
    end
    nil
  end

  # Handle a sig build validation failure. This allows users to override the
  # behavior when a sig build fails. If T::Configuration.sig_build_error_handler
  # is unset, this method will call handle_sig_build_error_default.
  def self.handle_sig_build_error(error, opts={})
    if T::Configuration.sig_build_error_handler
      T::Configuration.sig_build_error_handler.call(error, opts)
    else
      handle_sig_build_error_default(error, opts)
    end
    nil
  end

  # Handle a sig call validation failure. This allows users to override the
  # behavior when a sig call fails. If T::Configuration.call_validation_error_handler
  # is unset, this method will call handle_call_validation_error_default.
  def self.handle_call_validation_error(signature, opts={})
    if T::Configuration.call_validation_error_handler
      T::Configuration.call_validation_error_handler.call(signature, opts)
    else
      handle_call_validation_error_default(signature, opts)
    end
    nil
  end

  ### Default error handlers ###

  private_class_method def self.handle_type_error_default(type_error)
    raise type_error
  end

  private_class_method def self.handle_sig_decl_error_default(error, location)
    T::Private::Methods.sig_error(location, error.message)
  end

  private_class_method def self.handle_sig_build_error_default(error, opts)
    # if this method overrides a generated signature, report that one instead
    bad_method = opts[:method]
    if !opts[:declaration].generated
      super_signature = opts[:super_signature]
      raise error if !super_signature&.generated
      bad_method = super_signature.method
    end

    if defined?(Opus) && defined?(Opus::Log)
      method_file, method_line = bad_method.source_location
      Opus::Log.info(
        "SIG-DECLARE-FAILED",
        definition_file: method_file,
        definition_line: method_line,
        kind: "Delete",
        message: error.message,
        clevel: Chalk::Log::CLevels::Sheddable,
        )
    else
      puts "SIG-DECLARE-FAILED-WITHOUT-LOG Opus::Log is not included at this point. Please fix this sig: #{error.message}" # rubocop:disable PrisonGuard/NoBarePuts
    end
  end

  private_class_method def self.handle_call_validation_error_default(signature, opts)
    method_file, method_line = signature.method.source_location
    location = opts[:location]
    suffix = "Caller: #{location.path}:#{location.lineno}\n" \
               "Definition: #{method_file}:#{method_line}"

    error_message = "#{opts[:kind]}#{opts[:name] ? " '#{opts[:name]}'" : ''}: #{opts[:message]}\n#{suffix}"

    if signature.generated
      if defined?(Opus) && defined?(Opus::Log)
        got = opts[:value].class
        got = T.unsafe(T::Enumerable[T.untyped]).describe_obj(opts[:value]) if got < Enumerable
        Opus::Log.info(
          "SIG-CHECK-FAILED",
          caller_file: location.path,
          caller_line: location.lineno,
          definition_file: method_file,
          definition_line: method_line,
          kind: opts[:kind],
          name: opts[:name],
          expected: opts[:type].name,
          got: got,
          clevel: Chalk::Log::CLevels::Sheddable,
          )
      else
        puts "SIG-CHECK-FAILED-WITHOUT-LOG Opus::Log is not included at this point. Please fix this sig: #{error_message}" # rubocop:disable PrisonGuard/NoBarePuts
      end
    elsif signature.soft_notify
      if defined?(Opus) && defined?(Opus::Error)
        Opus::Error.soft("TypeError: #{error_message}", {notify: signature.soft_notify})
      else
        puts "TypeError: #{error_message}, notify: #{signature.soft_notify}" # rubocop:disable PrisonGuard/NoBarePuts
      end
    else
      begin
        raise TypeError.new(error_message)
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Private::ErrorHandler.handle_type_error(e)
      end
    end
  end
end
