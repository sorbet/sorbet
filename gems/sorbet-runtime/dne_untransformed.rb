module DNE
  LAZY_CONSTANT = begin puts '-- forcing LAZY_CONSTANT! --'; 0 end

  def self.main
    Integer::DoesNotExist
  end
end
