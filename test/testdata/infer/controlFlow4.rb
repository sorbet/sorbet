# @typed
module ModuleMethods

  def instrumented_request(final_attempt, foo)
    begin
    rescue StandardError => e
      err = e
    end

    is_successful = err.nil?
    if is_successful || final_attempt || (err && foo)
      if !is_successful
        1 # was-error: unrechable. Enabling a smarter algorithm in inference.cc classifies this as unreachable.
          # see 96ea3cacc6759fb2861252d51fcf3a148b85586d
          # this is because `e` is not assigned yet, is_successful is always true
      end
    end

  end
end
