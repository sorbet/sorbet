# typed: true

module A
  extend T::Sig

  module NotT
    def self.type_alias(&blk); end
  end

  NotATypeAlias = NotT.type_alias {Object}

  sig {returns(NotATypeAlias)}
  #            ^^^^^^^^^^^^^ error: Constant `A::NotATypeAlias` is not a class or type alias
  def this_has_an_invalid_return_type; end
end
