# typed: strict
class HasMeth
    def meth
        'meth'
    end
end

def returns_lambda
    lambda do |x| 'returns_lambda' end
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
    calls_arg_with_object(HasMeth.new, &blk)
end

class CallsWithObject
    def self.calls_with_object(&blk)
        blk.call(HasMeth.new)
    end
end

class CallsWithObjectChild < CallsWithObject
    def self.calls_with_object(&blk)
        super(&blk)
    end
end

def foo(&blk)
    calls_with_object(&:meth)
    calls_with_object {|*args| :meth.to_proc.call(*args)}
    calls_with_object {|x| :meth.to_proc.call(x)}
    calls_with_object {|x| x.meth}
    calls_with_object(&blk)
    calls_with_object(&returns_lambda)
    calls_with_object(&HasToProc.new)
    calls_with_object {|*args| HasToProc.new.to_proc.call(*args)}
    CallsWithObject.calls_with_object(&:meth)
    CallsWithObject&.calls_with_object(&:meth)
    CallsWithObjectChild.calls_with_object(&:meth)
    calls_arg_with_object(HasMeth.new, &:meth)
    calls_arg_with_object(HasMeth.new) {|x| x.meth}
end

foo {|x| 'foo'}
