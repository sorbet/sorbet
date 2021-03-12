# typed: strict

module First
  module Second
    module Third
      module Fourth
        extend T::Generic

        module NestedModule; end
        #      ^ show-symbol: First::Second::Third::Fourth::NestedModul
        NestedConstant = 1
        # ^ show-symbol: First::Second::Third::Fourth::NestedConstan
        NestedClassAlias = Integer
        # ^ show-symbol: First::Second::Third::Fourth::NestedClassAlia
        NestedTypeMember = type_member
        # ^ show-symbol: First::Second::Third::Fourth::NestedTypeMembe
      end
    end

    NestedTypeAlias = T.type_alias {Integer}
    # ^ show-symbol: First::Second::Third::NestedTypeAlia
  end
end
