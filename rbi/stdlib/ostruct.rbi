# typed: true

class OpenStruct
  InspectKey = ::T.let(nil, ::T.untyped)

  Sorbet.sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def ==(other); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](name); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(name, value); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def delete_field(name); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
      names: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def dig(name, *names); end

  Sorbet.sig {returns(::T.untyped)}
  def each_pair(); end

  Sorbet.sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def eql?(other); end

  Sorbet.sig {returns(::T.untyped)}
  def freeze(); end

  Sorbet.sig {returns(::T.untyped)}
  def hash(); end

  Sorbet.sig do
    params(
      hash: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(hash=T.unsafe(nil)); end

  Sorbet.sig {returns(::T.untyped)}
  def inspect(); end

  Sorbet.sig {returns(::T.untyped)}
  def marshal_dump(); end

  Sorbet.sig do
    params(
      x: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def marshal_load(x); end

  Sorbet.sig do
    params(
      mid: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def method_missing(mid, *args); end

  Sorbet.sig {returns(::T.untyped)}
  def modifiable(); end

  Sorbet.sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def new_ostruct_member(name); end

  Sorbet.sig {returns(::T.untyped)}
  def table(); end

  Sorbet.sig {returns(::T.untyped)}
  def table!(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_h(); end

  Sorbet.sig {returns(::T.untyped)}
  def to_s(); end
end
