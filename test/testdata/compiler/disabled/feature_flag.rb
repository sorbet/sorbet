# frozen_string_literal: true
# frozen_string_literal: true
# typed: true
# compiled: true
# compiled: true

module Opus; end
module Opus::Flag; end
module Opus::Flag::Private; end
module Opus::Flag::Private::Evaluation; end

module Opus::Flag
  module Private::Deserializer
    extend T::Sig

    # Deserialize an array of serialized FeatureFlag hashes.
    sig {params(flags_hash: T::Hash[T.untyped, T.untyped]).returns(T::Array[Private::Evaluation::FeatureFlag])}
    def self.deserialized_flags(flags_hash)
      flags_hash["flags"].map {|flag| Private::Evaluation::FeatureFlag.from_hash(flag)}
    end
  end
end

module Opus::Flag
  module Private::Evaluation::FeatureFlag
    def self.from_hash(flag)
    end
  end
end
