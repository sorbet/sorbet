# frozen_string_literal: true
# typed: false

module T::Private
  # Dynamically confirm that `value` is recursively a valid value of
  # type `type`, including recursively through collections. Note that
  # in some cases this runtime check can be very expensive, especially
  # with large collections of objects.
  def self.check_type_recursive!(value, type)
    T::Private::Casts.cast_recursive(value, type, cast_method: "T.check_type_recursive!")
  end

  module Casts
    def self.cast(value, type, cast_method:)
      begin
        error = T::Utils.coerce(type).error_message_for_obj(value)
        return value unless error

        caller_loc = T.must(caller_locations(2..2)).first

        suffix = "Caller: #{T.must(caller_loc).path}:#{T.must(caller_loc).lineno}"

        raise TypeError.new("#{cast_method}: #{error}\n#{suffix}")
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Configuration.inline_type_error_handler(e)
        value
      end
    end

    # there's a lot of shared logic with the above one, but factoring
    # it out like this makes it easier to hopefully one day delete
    # this one
    def self.cast_recursive(value, type, cast_method:)
      begin
        error = T::Utils.coerce(type).error_message_for_obj_recursive(value)
        return value unless error

        caller_loc = T.must(caller_locations(2..2)).first

        suffix = "Caller: #{T.must(caller_loc).path}:#{T.must(caller_loc).lineno}"

        raise TypeError.new("#{cast_method}: #{error}\n#{suffix}")
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Configuration.inline_type_error_handler(e)
        value
      end
    end
  end
end
