# typed: strict
def main
    begin
        a
    rescue
        b
    else
        c
    ensure
        d
    end
end

def a; end
def b; end
def c; end
def d; end

puts foo() # error: Method `foo` does not exist
