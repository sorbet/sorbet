# typed: strict

module Shellwords
  sig { params(str: String).returns(String) }
  def self.escape(str); end

  sig { params(str: T::Array[String]).returns(String) }
  def self.join(str); end

  sig { params(str: String).returns(String) }
  def self.shellescape(str); end

  sig { params(str: T::Array[String]).returns(String) }
  def self.shelljoin(str); end

  sig { params(str: String).returns(T::Array[String]) }
  def self.shellsplit(str); end

  sig { params(str: String).returns(T::Array[String]) }
  def self.shellwords(str); end

  sig { params(str: String).returns(T::Array[String]) }
  def self.split(str); end
end
