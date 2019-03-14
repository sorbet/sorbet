# typed: true
def bar; end;
foo += raise(bar) rescue nil # error: does not exist on `NilClass`
