# typed: true

# imagine this was pulled from sorbet-typed

class MyGem
  extend T::Sig

  sig {returns(NilClass)}
  def self.method_thats_part_of_my_public_api; end
end
