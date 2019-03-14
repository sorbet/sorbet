# typed: true
def m(a); end;
foo += m foo # error: Method `+` does not exist on `NilClass`
