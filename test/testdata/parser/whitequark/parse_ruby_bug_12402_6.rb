# typed: true
def foo; end;
def bar; end;
foo::C ||= raise(bar) rescue nil # error-with-dupes: Constant reassignment is not supported
