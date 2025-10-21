module Foo
  extend T::Sig

  sig { void }
  def self.method_1; end

  sig { void }
  private_class_method def self.method_2; end

  sig { void }
  def method_3; end

  protected

  sig { void }
  def method_4; end

  private

  sig { void }
  def method_5; end

  class Bar
    extend T::Sig

    class << self
      extend T::Sig

      sig { void }
      def method_6; end
    end

    sig { void }
    def self.method_7; end

    sig { void }
    private_class_method def self.method_8; end

    sig { void }
    def method_9; end

    private

    sig { returns(T.untyped) }
    def method_10; end

    protected

    sig { void }
    def method_11; end
  end
end
