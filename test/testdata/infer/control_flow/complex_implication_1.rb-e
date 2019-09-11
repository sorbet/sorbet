# typed: true
module ModuleMethods

  def instrumented_request(final_attempt, foo)
    begin
    rescue StandardError => e
      err = e
    end

    is_successful = err.nil?
    if is_successful || final_attempt || (err && foo) # error: This code is unreachable
    # Disabling a smarter algorithm in inference.cc will no longer classify this as unreachable.
    # this is because `e` is not assigned yet, is_successful is always true
      if !is_successful
        1
      end
    end

  end
end
