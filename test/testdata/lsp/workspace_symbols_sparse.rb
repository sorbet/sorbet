# typed: true

def fbb
  # ^^^ symbol-search: "fbb", rank=0
end

def fb_baz
  # ^^^^^^ symbol-search: "fbb", rank=10
end

def foo_bar_baz
  # ^^^^^^^^^^^ symbol-search: "fbb", rank=20
end

def other_foo_bar_baz
  # ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30
end

def xxfxxbxxbxx(xxx)
  # ^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=40
end

def fbnomatch
end

class A
  def fbb
    # ^^^ symbol-search: "fbb", rank=0
  end

  def fb_baz
    # ^^^^^^ symbol-search: "fbb", rank=10
  end

  def foo_bar_baz
    # ^^^^^^^^^^^ symbol-search: "fbb", rank=20
  end

  def other_foo_bar_baz
    # ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30
  end

  def xxfxxbxxbxx(xxx)
    # ^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=40
  end

  def fbnomatch
  end
end

module B
  def self.fbb
    #      ^^^ symbol-search: "fbb", rank=0
  end

  def self.fb_baz
    #      ^^^^^^ symbol-search: "fbb", rank=10
  end

  def self.foo_bar_baz
    #      ^^^^^^^^^^^ symbol-search: "fbb", rank=20
  end

  def self.other_foo_bar_baz
    #      ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30
  end

  def self.xxfxxbxxbxx(xxx)
    #      ^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=40
  end

  def self.fbnomatch
  end

  class C
    attr_reader :fbb
    #            ^^^ symbol-search: "fbb", rank=0

    attr_reader :fb_baz
    #            ^^^^^^ symbol-search: "fbb", rank=10

    attr_reader :foo_bar_baz
    #            ^^^^^^^^^^^ symbol-search: "fbb", rank=20

    attr_reader :other_foo_bar_baz
    #            ^^^^^^^^^^^ symbol-search: "fbb", rank=30
    
    attr_reader :xxfxxbxxbxx
    #            ^^^^^^^^^^^ symbol-search: "fbb", rank=40

    attr_reader :fbnomatch
  end

  class D
    def initialize
      @fbb = T.let(0, Integer)
      #^^^^^^^^^^^ symbol-search: "fbb", rank=0
      @fb_baz = T.let(1, Integer)
      #^^^^^^^^^^^ symbol-search: "fbb", rank=10
      @foo_bar_baz = T.let(2, Integer)
      #^^^^^^^^^^^ symbol-search: "fbb", rank=20
      @other_foo_bar_baz = T.let(3, Integer)
      #^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30
      @xxfxxbxxbxx = T.let(4, Integer)
      #^^^^^^^^^^^ symbol-search: "fbb", rank=40
      @fbnomatch = T.let(5, Integer)
    end
  end
end
