# typed: strict

extend T::Helpers

def hazTwoArgs(a, b); 1; end;

def baz
  if someCondition
    []
  else
    "[]"
  end
end

sig {void}
def give_me_void; end

def bla; give_me_void; end

def bbq
  if someCondition
    give_me_void
  else
    nil
  end
end

def idk(a); a / a + a * a; end

def give_me_literal; 1; end;

def give_me_literal_nested; [[1]]; end;

private def root_private; end

protected def root_protected; end

class A
  extend T::Helpers

  private def a_private; end

  protected def a_protected; end
end

def foo(a)
 1 + a
end

sig {params(a: Integer).void}
def takesInt(a); end;

sig {params(a: String).void}
def takesString(a); end;

def fooCond(a, cond)
  if cond
    takesInt(a)
  else
    takesString(a)
  end
end

def fooWhile(a, cond1, cond2)
  while cond2
    if cond1
      takesInt(a)
    else
      takesString(a)
    end
  end
end

def takesBlock
  yield 1
  2
end

def list_ints_or_empty_list
  x = T.let(1, T.nilable(Integer))
  x.nil? ? [x] : []
end

def dead(x)
  if true || qux || blah
    takesInt(x)
  else
    takesString(x)
  end
end

def with_block
  yield
  nil
end


def takesRepated(*a); end;
def hasNoName(*); end;

# We used to generate multiple sigs for dsl'd methods here
Foo = Struct.new(:a, :b)

class TestCarash
  class Merchant
  end

  sig {params(merchant: Merchant).void}
  def self.blar(merchant:)
  end

  def self.load_account_business_profile(merchant)
    blar(merchant: merchant)
    1
  end
end

def cantRun(a)
  takesInt(a)
  takesString(a)
  1
end

# Suggest "implementation." for these classes
module Abstract
  extend T::Helpers
  abstract!
  sig {abstract.void}
  def foo; end
end
class Implementation
  include Abstract

  def foo; end
end

# Suggest "override." for these classes
class Parent
  extend T::Helpers
  sig {void}
  def initialize; end

  sig {overridable.void}
  def foo; end

  sig {void}
  def bar; end

  def qux; end
end
class Child < Parent
  def initialize; end

  def foo; end

  def bar; end

  def qux; end
end

sig {params(a: T.untyped, cond: T.any(TrueClass, FalseClass)).returns(T.untyped).generated}
def fooCondGeneratedCurly(a, cond)
  if cond
    takesInt(a)
  else
    takesString(a)
  end
end


sig do
  params(a: T.untyped, cond: T.any(TrueClass, FalseClass)).
  returns(T.untyped).
  generated
end
def fooCondGeneratedDo(a, cond)
  if cond
    takesInt(a)
  else
    takesString(a)
  end
end
