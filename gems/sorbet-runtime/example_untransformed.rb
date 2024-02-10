module Example
  LAZY_CONSTANT = begin puts '-- forcing LAZY_CONSTANT! --'; 0 end

  def self.main
    T.must(nil)
  end
end
