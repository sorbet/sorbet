# @typed
class A < Exception; end
class B < Exception; end
def main
  try = 0
  begin
      puts "top"
      begin
        if try < 3 # error: Method < does not exist on NilClass component of NilClass | Integer
            try += 1 # error: Method + does not exist on NilClass component of NilClass | Integer
            raise A.new # error: MULTI
        elsif try < 6 # error: Method < does not exist on NilClass component of NilClass | Integer
            try += 1 # error: Method + does not exist on NilClass component of NilClass | Integer
            raise B.new # error: MULTI
        end
      rescue A
        puts "rescue A"
        retry
        1
      end
  rescue B # error: Argument arg0 does not match expected type
    puts "rescue B "
    retry
    2
  end
end
main
