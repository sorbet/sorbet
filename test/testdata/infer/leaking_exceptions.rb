# typed: true

class A < Exception; end
class B < Exception; end

begin
  raise A.new
rescue A => e
end
loop do
  begin
    puts e
  rescue B => e # error: Changing the type of a variable in a loop is not permitted
  end
end
