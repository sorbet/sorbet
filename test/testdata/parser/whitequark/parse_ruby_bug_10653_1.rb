# typed: true
def cond; end;
def tap; end;
cond ? raise {} : tap {} # error: This code is unreachable
