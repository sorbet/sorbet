# frozen_string_literal: true
# typed: strict

class Opus::Flatfiles
  class Record
    extend T::Sig

    sig {params(name: T.untyped, type: T.untyped).void}
    def self.dsl_required(name, type); end

    sig {params(blk: T.proc.void).void}
    def self.flatfile(&blk); end

    sig {params(name: T.untyped).void}
    def self.field(name); end

    sig {params(pattern: T.untyped, name: T.untyped).void}
    def self.pattern(pattern, name); end

    sig {params(pattern: T.untyped, name: T.untyped).void}
    def self.from(pattern, name); end
  end

  class MarkupLanguageNodeStruct
    extend T::Sig

    sig {params(blk: T.proc.void).void}
    def self.flatfile(&blk); end

    sig {params(name: T.untyped).void}
    def self.field(name); end
  end
end
