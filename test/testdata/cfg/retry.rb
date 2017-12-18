# @typed
def main
  try = 0
  begin
    if try < 3 # error: Method < does not exist on NilClass component of Integer | NilClass
        try += 1 # error: Method + does not exist on NilClass component of Integer | NilClass
        raise "e"
    end
  rescue
    puts "rescue"
    retry
    1
  end
end
main
