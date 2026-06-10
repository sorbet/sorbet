# frozen_string_literal: true
# typed: true

module T::Private
  module Casts
    def self.cast(value, type, cast_method)
      begin
        # Fast path for the common case where `type` is a Module literal
        # (e.g. `T.let(x, Integer)`). Mirrors the Module branch of
        # T::Utils::Private.coerce_and_check_module_types with
        # check_module_type=true: check the value against the module first,
        # and only build the pooled type object when that check fails.
        if ::Module === type
          return value if value.is_a?(type)

          coerced_type = T::Types::Simple::Private::Pool.type_for_module(type)
        elsif type.is_a?(T::Types::Base)
          # Mirrors the T::Types::Base branch of coerce_and_check_module_types,
          # kept inline to avoid its call frame for every already-coerced type.
          if type.instance_of?(T::Private::Types::SimplePairUnion)
            # The same idea for the unions of two plain modules that
            # `T.nilable(SomeModule)` (as well as `T.any` of two modules and
            # `T::Boolean`) produce: valid? is exactly two is_a? checks, so the
            # happy path can short-circuit the error-message wrapper dispatch.
            # instance_of? rather than is_a?, so that any hypothetical subclass
            # overriding valid? semantics takes the generic path below.
            return value if type.valid?(value)

            coerced_type = type
          elsif type.is_a?(T::Private::Types::TypeAlias)
            coerced_type = type.aliased_type
          else
            coerced_type = type
          end
        else
          coerced_type = T::Utils::Private.coerce_and_check_module_types(type, value, true)
          return value unless coerced_type
        end

        error = coerced_type.error_message_for_obj(value)
        return value unless error

        caller_loc = T.must(caller_locations(2..2)).first

        suffix = "Caller: #{T.must(caller_loc).path}:#{T.must(caller_loc).lineno}"

        raise TypeError.new("#{cast_method}: #{error}\n#{suffix}")
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Configuration.inline_type_error_handler(e, {kind: cast_method, value: value, type: type})
        value
      end
    end

    # there's a lot of shared logic with the above one, but factoring
    # it out like this makes it easier to hopefully one day delete
    # this one
    def self.cast_recursive(value, type, cast_method)
      begin
        error = T::Utils.coerce(type).error_message_for_obj_recursive(value)
        return value unless error

        caller_loc = T.must(caller_locations(2..2)).first

        suffix = "Caller: #{T.must(caller_loc).path}:#{T.must(caller_loc).lineno}"

        raise TypeError.new("#{cast_method}: #{error}\n#{suffix}")
      rescue TypeError => e # raise into rescue to ensure e.backtrace is populated
        T::Configuration.inline_type_error_handler(e, {kind: cast_method, value: value, type: type})
        value
      end
    end
  end
end
