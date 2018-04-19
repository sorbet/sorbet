# typed: strict
class Main
    sig.returns(nil) # error: You probably meant .returns(NilClass)
    def returns_nil; end

    sig.returns(nil) # error: You probably meant .returns(NilClass)
    def returns_nil_bad
        3 # error: Returning value that does not conform to method result type
    end
end
