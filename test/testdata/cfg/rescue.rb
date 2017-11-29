# @typed
def foo
    begin
        1
    rescue
        2
    else
        3
    end
end

puts foo()
