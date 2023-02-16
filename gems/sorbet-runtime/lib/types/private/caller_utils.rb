# frozen_string_literal: true
# typed: false

module T::Private::CallerUtils
  def self.find_caller(caller_locations)
    caller_locations.find do |loc|
      next if loc.path&.start_with?("<internal:")
      yield loc
    end
  end
end
