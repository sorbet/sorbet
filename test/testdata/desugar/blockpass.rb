class HasMeth
    def meth
        'meth'
    end
end

def returns_lambda
    lambda do |x| 'returns_lambda' end # error: Method lambda does not exist on Object
end

class HasToProc
    def to_proc
        returns_lambda
    end
end

def calls_arg_with_object(arg, &blk)
    blk.call(arg);
end

def calls_with_object(&blk)
    calls_arg_with_object(HasMeth.new, &blk) # error: Unsupported node type Splat
end

class CallsWithObject
    def self.calls_with_object(&blk)
        blk.call(HasMeth.new)
    end
end

class CallsWithObjectChild < CallsWithObject
    def self.calls_with_object(&blk)
        super(&blk) # error: Unsupported node type Splat
    end
end

def foo(&blk)
    calls_with_object(&:meth)
    calls_with_object {|*args| :meth.to_proc.call(*args)} # error: MULTI
    calls_with_object {|x| :meth.to_proc.call(x)} # error: Method call does not exist on Proc
    calls_with_object {|x| x.meth}
    calls_with_object(&blk) # error: Unsupported node type Splat
    calls_with_object(&returns_lambda) # error: Unsupported node type Splat
    calls_with_object(&HasToProc.new) # error: Unsupported node type Splat
    calls_with_object {|*args| HasToProc.new.to_proc.call(*args)} # error: Unsupported
    CallsWithObject.calls_with_object(&:meth)
    CallsWithObject&.calls_with_object(&:meth)
    CallsWithObjectChild.calls_with_object(&:meth)
    calls_arg_with_object(HasMeth.new, &:meth)
    calls_arg_with_object(HasMeth.new) {|x| x.meth}
end

foo {|x| 'foo'}
