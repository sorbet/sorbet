# typed: true

class Main
    extend T::Sig

    sig {returns(NilClass).soft(notify: 'pt')}
    def soft
    end

    # Since it is an experiement, all these illegal things are ok for now
    sig {returns(NilClass).soft.soft}
    def two_soft
    end
    sig {returns(NilClass).soft.checked(false)}
    def soft_not_checked
    end
    sig {returns(NilClass).checked(false).soft}
    def not_checked_soft
    end
    sig {returns(NilClass).soft}
    def soft_no_notify
    end
    sig {returns(NilClass).soft(notify: Object.new)}
    def soft_wtf_notify
    end
end
