# typed: true

class OpenStruct
  InspectKey = T.let(T.unsafe(nil), Symbol)

  def marshal_dump()
  end

  def marshal_load(x)
  end

  def each_pair()
  end

  def delete_field(name)
  end

  def ==(other)
  end

  def to_s()
  end

  def to_h()
  end

  def [](name)
  end

  def []=(name, value)
  end

  def eql?(other)
  end

  def dig(name, *names)
  end

  def freeze()
  end

  def inspect()
  end

  def hash()
  end

  def method_missing(mid, *args)
  end
end
