# frozen_string_literal: true
# typed: false

module T::Private
  module Casts
    def self.cast(value, type, cast_method:)
      begin
        error = T::Utils.coerce(type).error_message_for_obj(value)
        return value unless error

        caller_loc = T.must(caller_locations(2..2)).first

        suffix = "Caller: #{T.must(caller_loc).path}:#{T.must(caller_loc).lineno}"

        raise TypeError.new("#{cast_method}: #{error}\n#{suffix}")
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Private::ErrorHandler.handle_inline_type_error(e)
        value
      end
    end
  end
end
