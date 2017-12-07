# @typed
def a(foo, bar)
    begin
        return 1
    rescue
        a = 2
    end
    a + 3
end
