# @typed
def main
    begin
        a
    ensure
        b
    end
end

def a; end
def b; end

main
