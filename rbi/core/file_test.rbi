# typed: __STDLIB_INTERNAL

module FileTest
  # The file_name parameter for all of these methods can either be a String
  # or an IO object.
  FileName = T.type_alias(T.any(String, IO))

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.blockdev?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.chardev?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.directory?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.empty?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.executable?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.executable_real?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.exist?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.exists?(file_name); end

  sig { params(file: FileName).returns(T::Boolean) }
  def self.file?(file); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.grpowned?(file_name); end

  sig { params(file_1: FileName, file_2: FileName).returns(T::Boolean) }
  def self.identical?(file_1, file_2); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.owned?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.pipe?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.readable?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.readable_real?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.setgid?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.setuid?(file_name); end

  sig { params(file_name: FileName).returns(Integer) }
  def self.size(file_name); end

  sig { params(file_name: FileName).returns(T.nilable(Integer)) }
  def self.size?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.socket?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.sticky?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.symlink?(file_name); end

  sig { params(file_name: FileName).returns(T.nilable(Integer)) }
  def self.world_readable?(file_name); end

  sig { params(file_name: FileName).returns(T.nilable(Integer)) }
  def self.world_writable?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.writable?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.writable_real?(file_name); end

  sig { params(file_name: FileName).returns(T::Boolean) }
  def self.zero?(file_name); end
end
