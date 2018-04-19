# typed: strict
def foo
    1..2
    Range.new(1, 2)
    1...2
    Range.new(1, 2, true)
end
