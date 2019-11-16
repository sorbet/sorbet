# typed: true

T.reveal_type(self.new) # error: Revealed type: `T.class_of(<root>)::<AttachedClass>`

class A
  extend T::Sig

  sig {returns(T.experimental_attached_class)}
  def self.make_one
    T.reveal_type(self.new) # error: Revealed type: `T.class_of(A)::<AttachedClass>`
  end

  sig {returns(A)}
  def self.make_one2
    T.reveal_type(new) # error: Revealed type: `A`
  end

end
