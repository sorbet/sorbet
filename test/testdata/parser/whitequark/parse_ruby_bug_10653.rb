# typed: true
def p(arg); end;
def cond; end;
cond ? 1.tap do |n| p n end : 0 # error: Method `tap` does not exist
