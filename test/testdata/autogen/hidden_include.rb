# typed: true
module A
end

class B
  def self.init
    include A
  end
end
