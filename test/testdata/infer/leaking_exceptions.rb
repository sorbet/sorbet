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
  rescue B => e
  end
end
