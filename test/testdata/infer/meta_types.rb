# typed: true

# Test that putting a MetaType into various erroneous places doesn't
# crash us (and raises appropriate user-level errors)

class TestMetaType
  def _; end

  def testit
    puts(T::Array[String])

    puts(
      if _
        T::Array[String]
      else
        false
      end
    )

    puts(
          if _
            T::Array[String]
          else
            T::Array[Float]
          end
        )
  end
end
