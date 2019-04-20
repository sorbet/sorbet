# typed: true

class OpenStruct
  InspectKey = ::T.let(nil, ::T.untyped)

  sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(other); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](name); end

  sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(name, value); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def delete_field(name); end

  sig do
    params(
      name: ::T.untyped,
      names: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dig(name, *names); end

  sig {returns(::T.untyped)}
  def each_pair(); end

  sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(other); end

  sig {returns(::T.untyped)}
  def freeze(); end

  sig {returns(::T.untyped)}
  def hash(); end

  sig do
    params(
      hash: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(hash=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def inspect(); end

  sig {returns(::T.untyped)}
  def marshal_dump(); end

  sig do
    params(
      x: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def marshal_load(x); end

  sig do
    params(
      mid: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def method_missing(mid, *args); end

  sig {returns(::T.untyped)}
  def modifiable(); end

  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def new_ostruct_member(name); end

  sig {returns(::T.untyped)}
  def table(); end

  sig {returns(::T.untyped)}
  def table!(); end

  sig {returns(::T.untyped)}
  def to_h(); end

  sig {returns(::T.untyped)}
  def to_s(); end
end
