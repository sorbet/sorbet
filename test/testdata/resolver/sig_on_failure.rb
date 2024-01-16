# typed: true

class Main
    extend T::Sig

    sig {returns(NilClass).on_failure(notify: 'pt')}
    def on_failure
    end

    # Since it is an experiment, all these illegal things are ok for now
    sig {returns(NilClass).on_failure(notify: 'pt').on_failure(notify: 'pt')}
    def two_on_failure
    end
    sig {returns(NilClass).on_failure(notify: 'pt').checked(false)}
    def on_failure_not_checked
    end
    sig {returns(NilClass).checked(false).on_failure(notify: 'pt')}
    def not_checked_on_failure
    end
    sig {returns(NilClass).on_failure(notify: 'pt')}
    def on_failure_no_notify
    end
    sig {returns(NilClass).on_failure(notify: Object.new)}
    def on_failure_wtf_notify
    end
end
