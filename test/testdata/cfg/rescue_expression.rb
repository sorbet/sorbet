# typed: strict
class MyException < Exception
end

def foo
    begin
        raise MyException.new
    rescue MyException.new.class => e
        3
    end
end
puts foo
