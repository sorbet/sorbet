# @typed
class Main
    sig.returns(Junk) # error: Stubbing out unknown constant
    def foo
        Junk.new
    end
end
puts Main.new.foo
