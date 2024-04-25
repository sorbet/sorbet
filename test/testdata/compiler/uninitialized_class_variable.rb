# frozen_string_literal: true
# compiled: true
# typed: true

class A
  def self.init_change(change)
    @@changes ||= {}
    @@changes[change] = :done
  end

  def self.get_change(change)
    @@changes[change]
  end
end

A.init_change('constant')
A.init_change('other')
p A.get_change('constant')
p A.get_change('other')
