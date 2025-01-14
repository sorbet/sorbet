# typed: strict
extend T::Sig

sig { params(x: T.any(T::Array[Integer], T::Hash[String, Symbol])).void }
def example(x)
  x.map do |elem|
    #^ hover-line: 2 # Array#map (overload.1):
    #^ hover-line: 3 sig do
    #^ hover-line: 4   type_parameters(:U)
    #^ hover-line: 5   .params(
    #^ hover-line: 6     blk: T.proc.params(arg0: Integer).returns(T.type_parameter(:U))
    #^ hover-line: 7   )
    #^ hover-line: 8   .returns(T::Array[T.type_parameter(:U)])
    #^ hover-line: 9 end
    #^ hover-line: 10 def map (overload.1)(&blk); end
    #^ hover-line: 11 # Enumerable#map (overload.1):
    #^ hover-line: 12 sig do
    #^ hover-line: 13   type_parameters(:U)
    #^ hover-line: 14   .params(
    #^ hover-line: 15     blk: T.proc.params(arg0: [String, Symbol]).returns(T.type_parameter(:U))
    #^ hover-line: 16   )
    #^ hover-line: 17   .returns(T::Array[T.type_parameter(:U)])
    #^ hover-line: 18 end
    #^ hover-line: 19 def map (overload.1)(&blk); end
    #^ hover-line: 21 # result type:
    #^ hover-line: 22 T::Array[T.any(Integer, [String, Symbol])]
    T.reveal_type(elem) # error: `T.any(Integer, [String, Symbol])`
  end
end
