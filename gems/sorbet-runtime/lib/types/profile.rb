# typed: true
# frozen_string_literal: true

module T::Profile
  SAMPLE_RATE = 101 # 1 out of that many typechecks will be measured
  class <<self
    attr_accessor :typecheck_duration
    attr_accessor :typecheck_samples
    attr_accessor :typecheck_sample_attempts
    def typecheck_duration_estimate
      total_typechecks = typecheck_samples * SAMPLE_RATE + (SAMPLE_RATE - typecheck_sample_attempts)
      typechecks_measured = typecheck_samples * SAMPLE_RATE
      typecheck_duration * SAMPLE_RATE * 1.0 * total_typechecks / typechecks_measured
    end

    def typecheck_count_estimate
      typecheck_samples * SAMPLE_RATE
    end

    def reset
      @typecheck_duration = 0
      @typecheck_samples = 0
      @typecheck_sample_attempts = SAMPLE_RATE
    end
  end
  self.reset
end
