# typed: true

module Top
  class Nested
    include T::CFGExport
    Const = 30
    
    def self_send
      self_send
    end
  end
end

module Another; end

module F
  extend T::Sig
  include T::CFGExport
  include Kernel
  
  def literals
    int = 42
    str = "foo"
    sym = :bar
    btrue = true
    bfalse = false
    float = 3.14
  end

  sig {params(
    cl: T.class_of(Top::Nested),
    inst: Top::Nested,
    or1: T::Boolean,
    or2: T.nilable(String),
    or3: T.any(Top, Another),
    and1: T.all(Top, Another),
    shape: {a: String, b: Integer},
    tuple: [String, Integer],
    array: T::Array[Top::Nested],
    hash: T::Hash[String, Integer],
  ).void}
  def types(cl, inst, or1, or2, or3, and1, shape, tuple, array, hash)
    42
  end

  sig {params(i: Integer).returns(Integer)}
  def instructions(i)
    j = i # load arg, ident
    i = 42 # literal
    j = Top::Nested::Const # alias
    if i > 0 # conditional
      i = j - i # send
    end
    j.times { 0 } # send with block
    raise "foo" rescue 0 # unanalyzable
    return j # return
  end
end


class DoesNotHaveExportedMethods
  def foo
    "i am not exported as I don't inherit T::CFGExport"
  end
end
