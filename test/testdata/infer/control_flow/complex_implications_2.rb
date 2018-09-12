# typed: true

def foo(foo)
    if foo
        bar = foo.is_a?(StandardError)
        if bar
            e = foo
            err = e
        end
    end

    junk = err
    if err
        1
    end

end
