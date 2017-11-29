# @typed
def foo
    begin
    rescue NotAClass => e # error: Stubbing out unknown constant <emptyTree>::<constant:NotAClass>
    else
    end
end
