# frozen_string_literal: true
# typed: true

module T::Types
  class TypedHash < TypedEnumerable
    # Technically we don't need these, but they are a nice api
    attr_reader :keys, :values

    def initialize(keys:, values:)
      @keys = T::Utils.coerce(keys)
      @values = T::Utils.coerce(values)
      @type = T::Utils.coerce([keys, values])
    end

    # @override Base
    def name
      "T::Hash[#{@keys.name}, #{@values.name}]"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Hash) && super
    end

    def new(*args, &blk) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
      Hash.new(*T.unsafe(args), &blk) # rubocop:disable PrisonGuard/RestrictHashDefaults
    end
  end
end
