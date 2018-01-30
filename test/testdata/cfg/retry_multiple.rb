# @typed
class A < Exception; end
class B < Exception; end
def main
  try = 0
  begin
    if try < 3 # error: Method < does not exist on NilClass component of T.any(NilClass, Integer)
        try += 1 # error: Method + does not exist on NilClass component of T.any(NilClass, Integer)
        raise A.new # error: MULTI
    elsif try < 6 # error: Method < does not exist on NilClass component of T.any(NilClass, Integer)
        try += 1 # error: Method + does not exist on NilClass component of T.any(NilClass, Integer)
        raise B.new # error: MULTI
    end
  rescue A
    puts "rescue A "
    retry
    1
  rescue B
    puts "rescue B "
    retry
    2
  end
end
main
