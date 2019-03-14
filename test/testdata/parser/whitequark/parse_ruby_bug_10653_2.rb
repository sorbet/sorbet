# typed: true
def cond; end;
def tap; end;
cond ? raise do end : tap do end # error: This code is unreachable
