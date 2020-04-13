# typed: strict

sig {params(file_name: String).returns(T::Array[String])}
def test_io_readlines_chomp(file_name)
  IO.readlines(file_name, chomp: true)
end
