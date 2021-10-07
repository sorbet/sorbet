# frozen_string_literal: true
# typed: true
# compiled: true

def test1; end
p method(:test1).parameters

def test2(a); end
p method(:test2).parameters

def test3(a, b=10); end
p method(:test3).parameters

def test4(a:); end
p method(:test4).parameters

def test5(a:, b: 10); end
p method(:test4).parameters

def test6(*rest); end
p method(:test6).parameters

def test7(**kwrest); end
p method(:test7).parameters

def test8(a, b:); end
p method(:test8).parameters

def test9(a, b=10, c:); end
p method(:test9).parameters

def test10(a, b:, c: 10); end
p method(:test10).parameters

def test11(a, b=10, c:, d: 10); end
p method(:test11).parameters

def test12(*rest, a:, b: 10); end
p method(:test12).parameters

def test13(a, b=10, *rest, c:, d: 10); end
p method(:test13).parameters

def test14(a, b=10, *rest, c:, d: 10, **kwrest); end
p method(:test14).parameters
