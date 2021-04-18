# typed: true
# frozen_string_literal: true

# Deprecated, kept only for partial backwards compatibility
module T::Profile
  SAMPLE_RATE = 101 # 1 out of that many typechecks will be measured
  class <<self
    attr_accessor :typecheck_duration
    attr_accessor :typecheck_samples
    attr_accessor :typecheck_sample_attempts
    def typecheck_duration_estimate
      0.0
    end

    def typecheck_count_estimate
      0
    end

    def reset
      @typecheck_duration = 0
      @typecheck_samples = 0
      @typecheck_sample_attempts = 0
    end
  end
  self.reset
end
