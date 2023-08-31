# typed: strict

def top_level; end

# Should only add one extend
class SuggestSigAndExtend
  def foo; end

  def bar; end
end

# This doesn't work right now, for both the injected 'sig' and 'extend'.
class SigAndExtendOneLine; def foo; end; end

class DontSuggestExtend
  extend T::Sig
  def foo; end
end

class Parent; end
class Child < Parent
  def foo; end
end

def project(x); x; end

class MethodBetweenExtendAndSig
  project :sorbet
  def foo; end
end

class MethodOnSelf
  def self.foo; end
end

# Should properly indent it when the first line has an empty line
class ProperIndentation

  def foo; end

  class Nested

    def bar; end
  end
end
