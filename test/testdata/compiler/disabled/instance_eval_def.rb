# frozen_string_literal: true
# typed: true
# compiled: true

module M
  include Kernel

  def self.included(klass)
    puts "hello from M.included"

    klass.instance_eval do
      puts "hello from the instance_eval block"
      puts "self is: #{self}"
      puts "let's def f"

      def f
        puts "hello from f"
      end

      puts "done deffing f"
    end
  end
end

module Z
  include M
end

T.unsafe(Z).f
