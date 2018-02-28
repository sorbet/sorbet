# @typed
class A < Exception; end
class B < Exception; end
def main
  try = 0
  begin
    if try < 3
        try += 1
        raise A.new
    elsif try < 6
        try += 1
        raise B.new
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
