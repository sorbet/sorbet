# @typed
def main
  try = 0
  begin
    if try < 3
        try += 1
        raise "e"
    end
  rescue
    puts "rescue"
    retry
    1
  end
end
main
