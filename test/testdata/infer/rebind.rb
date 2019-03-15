# typed: true
class C
  def only_on_C
  end
end

class B
  extend T::Sig
  sig {params(blk: T.proc.bind(C).void).void}
  def only_on_B(&blk)
  end
end

class A
  extend T::Sig
  sig {params(blk: T.proc.bind(B).void).void}
  def self.mySig(&blk)
  end
end

class Use
  extend T::Sig

  def only_on_Use
  end

  def shouldRemoveSelfTemp
    only_on_Use do
      1
    end
  end

  def jumpBetweenClasses
    A.mySig do
      only_on_Use # error: does not exist
      mySig # error: does not exist
      only_on_B do
        only_on_B # error: does not exist
        only_on_C
      end
    end
  end
end
