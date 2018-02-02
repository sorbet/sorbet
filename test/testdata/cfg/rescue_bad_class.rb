# @typed
def foo
    begin
    rescue NotAClass => e # error: Stubbing out unknown constant
    else
    end
end
