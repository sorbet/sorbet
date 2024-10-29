# typed: strict
extend T::Sig

sig { params(x: T.any(T::Array[Integer], T::Hash[String, Symbol])).void }
def example(x)
  x.map do |elem|
    #^ hover-line: 2 # Array#map:
    #^ hover-line: 3 sig do
    #^ hover-line: 4   params(
    #^ hover-line: 5     blk: T.proc.params(arg0: Integer).returns(T.type_parameter(:U))
    #^ hover-line: 6   )
    #^ hover-line: 7   .returns(T::Array[T.type_parameter(:U)])
    #^ hover-line: 8 end
    #^ hover-line: 9 def map(&blk); end
    #^ hover-line: 10 # Enumerable#map:
    #^ hover-line: 11 sig do
    #^ hover-line: 12   params(
    #^ hover-line: 13     blk: T.proc.params(arg0: [String, Symbol]).returns(T.type_parameter(:U))
    #^ hover-line: 14   )
    #^ hover-line: 15   .returns(T::Array[T.type_parameter(:U)])
    #^ hover-line: 16 end
    #^ hover-line: 17 def map(&blk); end
    #^ hover-line: 19 # result type:
    #^ hover-line: 20 T::Array[T.any(Integer, [String, Symbol])]
    T.reveal_type(elem) # error: `T.any(Integer, [String, Symbol])`
  end
end
