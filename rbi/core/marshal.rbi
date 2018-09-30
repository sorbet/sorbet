# typed: true
module Marshal
  MAJOR_VERSION = T.let(T.unsafe(nil), Integer)
  MINOR_VERSION = T.let(T.unsafe(nil), Integer)

  sig do
    params(
        arg0: Object,
        arg1: IO,
        arg2: Integer,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Object,
        arg1: Integer,
    )
    .returns(Object)
  end
  def self.dump(arg0, arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
        arg1: Proc,
    )
    .returns(Object)
  end
  def self.load(arg0, arg1=T.unsafe(nil)); end
end
