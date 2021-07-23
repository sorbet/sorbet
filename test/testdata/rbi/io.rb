# typed: strict

extend T::Sig

sig {params(file_name: String).returns(T::Array[String])}
def test_io_readlines_chomp(file_name)
  IO.readlines(file_name, chomp: true)
end

sig {params(file_name: String).void}
def test_io_read_encoding(file_name)
  IO.read(file_name, encoding: "ASCII_8BIT")
  IO.read(file_name, encoding: Encoding::ASCII_8BIT)
  IO.read(file_name, encoding: Encoding::ASCII_8BIT.to_s)
  IO.read(file_name, encoding: 42)
  #                            ^^ error: Expected `T.any(String, Encoding)` but found `Integer(42)` for argument `encoding`
end
