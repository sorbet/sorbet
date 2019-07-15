# typed: true
# disable-fast-path: true
class Base
  extend T::Sig

  sig do
    overridable
      .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
      .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwopt: nil, &blk); end
end

# exact match
class Ok_Exact < Base
  sig do
    override
    .overridable
    .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
    .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwopt: nil, &blk); end
end

# Additional (optional) positional args and kwargs
class Ok_Additional < Base
  sig do
    override
      .params(req: Object, opt: Object, opt2: Object, kwreq: Object, kwopt: Object, kwopt2: Object, blk: Proc)
      .returns(Object)
  end
  def foo(req, opt=nil, opt2=nil, kwreq:, kwopt: nil, kwopt2: nil, &blk); end
end

# Make required args optional
class Ok_Optional < Base
  sig do
    override
      .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
      .returns(Object)
  end
  def foo(req=nil, opt=nil, kwreq: nil, kwopt: nil, &blk); end
end

class Ok_BlockRenamed < Base
  sig do
    override
      .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, my_blk: Proc)
      .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwopt: nil, &my_blk); end
end

class Bad_MissingBlock < Base
  sig do
    override
      .params(req: Object, opt: Object, kwreq: Object, kwopt: Object)
      .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwopt: nil); end  # error: must explicitly name a block argument
end

class Bad_MissingPositional < Base
  sig do
    override
      .params(req: Object, kwreq: Object, kwopt: Object, blk: Proc)
      .returns(Object)
  end
  def foo(req, kwreq:, kwopt: nil, &blk); end  # error: must accept at least `2` positional arguments
end

class Bad_MissingKwarg < Base
  sig do
    override
    .params(req: Object, opt: Object, kwopt: Object, blk: Proc)
    .returns(Object)
  end
  def foo(req, opt=nil, kwopt: nil, &blk); end # error: missing required keyword argument `kwreq`
end

class Bad_ExtraKwarg < Base
  sig do
    override
    .params(req: Object, opt: Object, kwreq: Object, kwreq1: Object, kwopt: Object, blk: Proc)
    .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwreq1:, kwopt: nil, &blk); end # error: extra required keyword argument `kwreq1`
end

# Allows overrides if the parent isn't abstract or overridable
class Base1
  extend T::Sig

  sig do
    params(x: Integer, y: Integer).returns(Object)
  end
  def foo(x, y); end
end

class Ok_NotAbstract < Base1
  extend T::Sig

  sig do
    params(x: Integer).returns(Object)
  end
  def foo(x); end # Ok; don't check if not .overridable / .abstract
end

class Bad_Grandchild < Ok_Exact
  sig do
    override
    .params(req: Object, req1: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
    .returns(Object)
  end
  def foo(req, req1, opt=nil, kwreq:, kwopt: nil, &blk); end # error: must accept no more than `1` required argument
end

class Base_Repeated
  extend T::Sig

  sig do
    overridable
      .params(rest: Object, kwrest: Object)
      .returns(Object)
  end
  def foo(*rest, **kwrest)
  end
end

class Bad_NoRest < Base_Repeated
  sig do
    override
      .params(kwrest: Object)
      .returns(Object)
  end
  def foo(**kwrest) # error: must accept *`rest`
  end
end

class Bad_NoKwrest < Base_Repeated
  sig do
    override
      .params(rest: Object)
      .returns(Object)
  end
  def foo(*rest) # error: must accept **`kwrest`
  end
end


class NoOverride < Base
  sig do
      params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
      .returns(Object)
  end
  def foo(req, opt=nil, kwreq:, kwopt: nil, &blk); end # error: Method `NoOverride#foo` overrides `Base#foo` but is not declared with `.override`
end
