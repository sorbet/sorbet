# typed: false
class Test
  # Test that we resolve inside next/break (mostly a test of TreeMap,
  # which used to not recurse into these)
  def test_next_break
    each do
      if rand
        next Foo # error: Unable to resolve constant
      end
      if rand
        break Bar # error: Unable to resolve constant
      end
    end
  end
end
