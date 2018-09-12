# typed: strict
def foo
    a = 1..2
    b = Range.new(1, 2)
    c = 1...2
    d = Range.new(1, 2, true)
    e = (1..42).first
    f = ('a'..'z').last
end
