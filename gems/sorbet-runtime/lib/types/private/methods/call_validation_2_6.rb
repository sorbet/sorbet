  def self.create_validator_method_fast(mod, original_method, method_sig)
    if method_sig.return_type.is_a?(T::Private::Types::Void)
      raise "Should have used create_validator_procedure_fast"
    end
    # trampoline to reduce stack frame size
    if method_sig.arg_types.empty?
      create_validator_method_fast0(mod, original_method, method_sig, method_sig.return_type.raw_type)
    elsif method_sig.arg_types.length == 1
      create_validator_method_fast1(mod, original_method, method_sig, method_sig.return_type.raw_type,
                                    method_sig.arg_types[0][1].raw_type)
    elsif method_sig.arg_types.length == 2
      create_validator_method_fast2(mod, original_method, method_sig, method_sig.return_type.raw_type,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type)
    elsif method_sig.arg_types.length == 3
      create_validator_method_fast3(mod, original_method, method_sig, method_sig.return_type.raw_type,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type,
                                    method_sig.arg_types[2][1].raw_type)
    elsif method_sig.arg_types.length == 4
      create_validator_method_fast4(mod, original_method, method_sig, method_sig.return_type.raw_type,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type,
                                    method_sig.arg_types[2][1].raw_type,
                                    method_sig.arg_types[3][1].raw_type)
    else
      raise "should not happen"
    end
  end

  def self.create_validator_method_fast0(mod, original_method, method_sig, return_type)
    mod.send(:define_method, method_sig.method_name) do |&blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`
      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      return_value = original_method.bind(self).call(&blk)
      if should_sample
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless return_value.is_a?(return_type)
        message = method_sig.return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            return_type,
            return_value,
            caller_offset: -1
          )
        end
      end
      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end
      return_value
    end
  end

  def self.create_validator_method_fast1(mod, original_method, method_sig, return_type, arg0_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      return_value = original_method.bind(self).call(arg0, &blk)
      if should_sample
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless return_value.is_a?(return_type)
        message = method_sig.return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            method_sig.return_type,
            return_value,
            caller_offset: -1
          )
        end
      end
      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end
      return_value
    end
  end

  def self.create_validator_method_fast2(mod, original_method, method_sig, return_type, arg0_type, arg1_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      return_value = original_method.bind(self).call(arg0, arg1, &blk)
      if should_sample
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless return_value.is_a?(return_type)
        message = method_sig.return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            method_sig.return_type,
            return_value,
            caller_offset: -1
          )
        end
      end
      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end
      return_value
    end
  end

  def self.create_validator_method_fast3(mod, original_method, method_sig, return_type, arg0_type, arg1_type, arg2_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, arg2, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      unless arg2.is_a?(arg2_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[2][1].error_message_for_obj(arg2),
          'Parameter',
          method_sig.arg_types[2][0],
          arg2_type,
          arg2,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      return_value = original_method.bind(self).call(arg0, arg1, arg2, &blk)
      if should_sample
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless return_value.is_a?(return_type)
        message = method_sig.return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            method_sig.return_type,
            return_value,
            caller_offset: -1
          )
        end
      end
      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end
      return_value
    end
  end

  def self.create_validator_method_fast4(mod, original_method, method_sig, return_type,
      arg0_type, arg1_type, arg2_type, arg3_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, arg2, arg3, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      unless arg2.is_a?(arg2_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[2][1].error_message_for_obj(arg2),
          'Parameter',
          method_sig.arg_types[2][0],
          arg2_type,
          arg2,
          caller_offset: -1
        )
      end

      unless arg3.is_a?(arg3_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[3][1].error_message_for_obj(arg3),
          'Parameter',
          method_sig.arg_types[3][0],
          arg3_type,
          arg3,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      return_value = original_method.bind(self).call(arg0, arg1, arg2, arg3, &blk)
      if should_sample
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless return_value.is_a?(return_type)
        message = method_sig.return_type.error_message_for_obj(return_value)
        if message
          CallValidation.report_error(
            method_sig,
            message,
            'Return value',
            nil,
            method_sig.return_type,
            return_value,
            caller_offset: -1
          )
        end
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end
      return_value
    end
  end

  def self.create_validator_procedure_fast(mod, original_method, method_sig)
    # trampoline to reduce stack frame size
    if method_sig.arg_types.empty?
      create_validator_procedure_fast0(mod, original_method, method_sig)
    elsif method_sig.arg_types.length == 1
      create_validator_procedure_fast1(mod, original_method, method_sig,
                                    method_sig.arg_types[0][1].raw_type)
    elsif method_sig.arg_types.length == 2
      create_validator_procedure_fast2(mod, original_method, method_sig,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type)
    elsif method_sig.arg_types.length == 3
      create_validator_procedure_fast3(mod, original_method, method_sig,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type,
                                    method_sig.arg_types[2][1].raw_type)
    elsif method_sig.arg_types.length == 4
      create_validator_procedure_fast4(mod, original_method, method_sig,
                                    method_sig.arg_types[0][1].raw_type,
                                    method_sig.arg_types[1][1].raw_type,
                                    method_sig.arg_types[2][1].raw_type,
                                    method_sig.arg_types[3][1].raw_type)
    else
      raise "should not happen"
    end
  end

  def self.create_validator_procedure_fast0(mod, original_method, method_sig)
    mod.send(:define_method, method_sig.method_name) do |&blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      original_method.bind(self).call(&blk)
      T::Private::Types::Void::VOID
    end
  end

  def self.create_validator_procedure_fast1(mod, original_method, method_sig, arg0_type)

    mod.send(:define_method, method_sig.method_name) do |arg0, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it
      original_method.bind(self).call(arg0, &blk)
      T::Private::Types::Void::VOID
    end
  end

  def self.create_validator_procedure_fast2(mod, original_method, method_sig, arg0_type, arg1_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      original_method.bind(self).call(arg0, arg1, &blk)
      T::Private::Types::Void::VOID
    end
  end

  def self.create_validator_procedure_fast3(mod, original_method, method_sig, arg0_type, arg1_type, arg2_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, arg2, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      unless arg2.is_a?(arg2_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[2][1].error_message_for_obj(arg2),
          'Parameter',
          method_sig.arg_types[2][0],
          arg2_type,
          arg2,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      original_method.bind(self).call(arg0, arg1, arg2, &blk)
      T::Private::Types::Void::VOID
    end
  end

  def self.create_validator_procedure_fast4(mod, original_method, method_sig,
    arg0_type, arg1_type, arg2_type, arg3_type)
    mod.send(:define_method, method_sig.method_name) do |arg0, arg1, arg2, arg3, &blk|
      # This block is called for every `sig`. It's critical to keep it fast and
      # reduce number of allocations that happen here.
      # This method is a manually sped-up version of more general code in `validate_call`

      T::Profile.typecheck_sample_attempts -= 1
      should_sample = T::Profile.typecheck_sample_attempts == 0
      if should_sample
        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE
        T::Profile.typecheck_samples += 1
        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      end

      unless arg0.is_a?(arg0_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[0][1].error_message_for_obj(arg0),
          'Parameter',
          method_sig.arg_types[0][0],
          arg0_type,
          arg0,
          caller_offset: -1
        )
      end

      unless arg1.is_a?(arg1_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[1][1].error_message_for_obj(arg1),
          'Parameter',
          method_sig.arg_types[1][0],
          arg1_type,
          arg1,
          caller_offset: -1
        )
      end

      unless arg2.is_a?(arg2_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[2][1].error_message_for_obj(arg2),
          'Parameter',
          method_sig.arg_types[2][0],
          arg2_type,
          arg2,
          caller_offset: -1
        )
      end

      unless arg3.is_a?(arg3_type)
        CallValidation.report_error(
          method_sig,
          method_sig.arg_types[3][1].error_message_for_obj(arg3),
          'Parameter',
          method_sig.arg_types[3][0],
          arg3_type,
          arg3,
          caller_offset: -1
        )
      end

      if should_sample
        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)
      end

      # The following line breaks are intentional to show nice pry message










      # PRY note:
      # this code is sig validation code.
      # Please issue `finish` to step out of it

      original_method.bind(self).call(arg0, arg1, arg2, arg3, &blk)
      T::Private::Types::Void::VOID
    end
  end

