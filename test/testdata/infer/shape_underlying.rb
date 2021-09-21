# typed: true

T.let({foo: 0}, T::Hash[String, Float]) # error: Argument does not have asserted type
