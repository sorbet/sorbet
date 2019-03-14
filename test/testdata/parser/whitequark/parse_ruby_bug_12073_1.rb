# typed: true
class A::B; end;
def foo raise; raise A::B, ''; end
