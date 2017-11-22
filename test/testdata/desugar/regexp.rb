def foo
    /abc/ =~ "abcd"
    Regexp.new('abc') =~ "abcd"

    /abc/ix =~ "abcd"
    Regexp.new('abc', 'ix') =~ "abcd"

    a = "a"
    c = "c"
    /#{a}b#{c}/ =~ "abcd"
    Regexp.new(a + 'b' + c) =~ "abcd"
end
