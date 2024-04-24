# frozen_string_literal: true
# typed: strict

class Flatfiles::MyFlatfile < Opus::Flatfiles::Record
  dsl_required :deprecated?, String

  flatfile do
    from   1..2, :foo
    pattern(/A-Za-z/, :bar)
    field :baz
  end
end

class Flatfiles::MyXMLNode < Opus::Flatfiles::MarkupLanguageNodeStruct
  flatfile do
    field :quux
  end
end
