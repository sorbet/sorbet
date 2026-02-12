# NOTE: we don't actually validate `min_typed_level` in Sorbet, but if that
# changes in the future, this sigil should produce an error.
#
# typed: true

module Test::Root
  class ATest
    def test_a
      Root::A.new
    end
  end
end
