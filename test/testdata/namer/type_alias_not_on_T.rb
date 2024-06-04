# typed: true
module A
  module NotT
    def self.type_alias(&blk); end
  end

  ValidTypeAlias = T.type_alias {Object}

  NotActuallyATypeAlias = NotT.type_alias {Object}
end

module KnownLimitation
  module T
    def self.type_alias(&blk); end
  end

  # Known limitation: we can't distinguish between `::KnownLimitation::T` and `::T` in the namer,
  # so this will have a false positive, being treated as a type alias when it shouldn't be.
  StillATypeAlias = T.type_alias {Object}
end
