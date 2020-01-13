# typed: true

def fbb; end
#   ^^^ symbol-search: "fbb", rank=0

def fb_baz; end
#   ^^^^^^ symbol-search: "fbb", rank=10

def foo_bar_baz; end
#   ^^^^^^^^^^^ symbol-search: "fbb", rank=20

def other_foo_bar_baz; end
#   ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30

def xfxbxbx; end
#   ^^^^^^^ symbol-search: "fbb", rank=40

def fbnomatch; end

class A
  def fbb; end
  #   ^^^ symbol-search: "fbb", rank=0

  def fb_baz; end
  #   ^^^^^^ symbol-search: "fbb", rank=10

  def foo_bar_baz; end
  #   ^^^^^^^^^^^ symbol-search: "fbb", rank=20

  def other_foo_bar_baz; end
  #   ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30

  def xfxbxbx; end
  #   ^^^^^^^ symbol-search: "fbb", rank=40

  def fbnomatch; end
end

module B
  def self.fbb; end
  #        ^^^ symbol-search: "fbb", rank=0

  def self.fb_baz; end
  #        ^^^^^^ symbol-search: "fbb", rank=10

  def self.foo_bar_baz; end
  #        ^^^^^^^^^^^ symbol-search: "fbb", rank=20

  def self.other_foo_bar_baz; end
  #        ^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30

  def self.xfxbxbx; end
  #        ^^^^^^^ symbol-search: "fbb", rank=40

  def self.fbnomatch; end

  class C
    attr_reader :fbb
    #            ^^^ symbol-search: "fbb", rank=0
    attr_reader :fb_baz
    #            ^^^^^^ symbol-search: "fbb", rank=10
    attr_reader :foo_bar_baz
    #            ^^^^^^^^^^^ symbol-search: "fbb", rank=20
    attr_reader :other_foo_bar_baz
    #            ^^^^^^^^^^^ symbol-search: "fbb", rank=30
    attr_reader :xfxbxbx
    #            ^^^^^^^ symbol-search: "fbb", rank=40
    attr_reader :fbnomatch
  end

  class D
    def initialize
      @fbb = T.let(0, Integer)
      #^^^ symbol-search: "fbb", rank=0
      @fb_baz = T.let(1, Integer)
      #^^^^^^ symbol-search: "fbb", rank=10
      @foo_bar_baz = T.let(2, Integer)
      #^^^^^^^^^^^ symbol-search: "fbb", rank=20
      @other_foo_bar_baz = T.let(3, Integer)
      #^^^^^^^^^^^^^^^^^ symbol-search: "fbb", rank=30
      @xfxbxbx = T.let(4, Integer)
      #^^^^^^^ symbol-search: "fbb", rank=40
      @fbnomatch = T.let(5, Integer)
    end
  end
end
