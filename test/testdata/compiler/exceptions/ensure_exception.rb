# frozen_string_literal: true
# typed: true
# compiled: true


puts "==="
puts "Ensure gets the raised exception"
begin
  begin
    raise 'raised from body'
  ensure
    p $!
  end
rescue
  puts "Rescued from #{$!}"
end

puts "==="
puts "Ensure gets the excepton raised in rescue"
begin
  begin
    raise 'raised from foo'
  rescue
    raise 'raised from rescue'
  ensure
    p $!
  end
rescue
  puts "Rescued from #{$!}"
end

puts "==="
puts "Ensure gets the excepton raised from else"
begin
  begin
    puts "no exception raised in body"
  rescue
    puts "unexpected exception caught in rescue!"
  else
    raise 'raised from else'
  ensure
    p $!
  end
rescue
  puts "Rescued from #{$!}"
end
