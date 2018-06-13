# typed: strict

# Test that putting a MataType into various erroneous places doesn't
# crash us (and raises appropriate user-level errors)

class TestMetaType
  def _; end

  def testit
    puts(T::Array[String]) # error: `<Type: T::Array[String]>` doesn't match `BasicObject` for argument `arg0`

    puts(
      if _
        T::Array[String] # error: Unsupported usage of bare type
      else
        false
      end
    )
  end
end
