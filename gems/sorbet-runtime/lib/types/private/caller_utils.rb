# frozen_string_literal: true
# typed: false

module T::Private::CallerUtils
  if Thread.respond_to?(:each_caller_location) # RUBY_VERSION >= "3.2"
    def self.find_caller(callers_to_skip: 0)
      remaining_to_skip = callers_to_skip + 1
      Thread.each_caller_location do |loc|
        if remaining_to_skip.positive?
          remaining_to_skip -= 1
          next
        end

        next if loc.path&.start_with?("<internal:")

        return loc if yield(loc)
      end
      nil
    end
  else
    def self.find_caller(callers_to_skip: 0)
      caller_locations(2 + callers_to_skip).find do |loc|
        !loc.path&.start_with?("<internal:") && yield(loc)
      end
    end
  end
end
