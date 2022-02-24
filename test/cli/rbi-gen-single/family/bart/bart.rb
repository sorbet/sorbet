# typed: true

module Family
  module Bart

    class Character
      extend T::Sig

      # Ensure that lookup in a parent namespace works correctly. This should
      # show up as `Family::Simpsons` in the resulting rbi.
      sig {returns(T.class_of(Simpsons))}
      def family
        Simpsons
      end

      sig {void}
      def catchphrase
        Util::Messages.say "eat pant"
      end

      FamilyClass = Simpsons

    end

  end
end
