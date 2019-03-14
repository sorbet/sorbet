# typed: true

undef foo, :bar, :"foo#{1}" # error: Unsupported node type `Undef`
