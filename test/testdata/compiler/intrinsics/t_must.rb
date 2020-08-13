# frozen_string_literal: true
# typed: true
# compiled: true

module Test
  def self.test_known_nil
    begin
      T.must(nil)
    rescue => e
      puts e
    end
  end

  def self.test_nilable_arg(arg)
    begin
      T.must(arg)
      puts "#{arg} wasn't nil"
    rescue => e
      puts e
    end
  end
end

Test.test_known_nil
Test.test_nilable_arg(10)
Test.test_nilable_arg(false)
Test.test_nilable_arg(nil)
