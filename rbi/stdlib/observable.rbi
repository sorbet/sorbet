# typed: __STDLIB_INTERNAL

# The Observer pattern (also known as publish/subscribe) provides a simple
# mechanism for one object to inform a set of interested third-party objects
# when its state changes.
#
# ## Mechanism
#
# The notifying class mixes in the `Observable` module, which provides the
# methods for managing the associated observer objects.
#
# The observable object must:
# *   assert that it has `#changed`
# *   call `#notify_observers`
#
#
# An observer subscribes to updates using
# [`Observable#add_observer`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-add_observer),
# which also specifies the method called via
# [`notify_observers`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-notify_observers).
# The default method for
# [`notify_observers`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-notify_observers)
# is update.
#
# ### Example
#
# The following example demonstrates this nicely. A `Ticker`, when run,
# continually receives the stock `Price` for its `@symbol`. A `Warner` is a
# general observer of the price, and two warners are demonstrated, a `WarnLow`
# and a `WarnHigh`, which print a warning if the price is below or above their
# set limits, respectively.
#
# The `update` callback allows the warners to run without being explicitly
# called. The system is set up with the `Ticker` and several observers, and the
# observers do their duty without the top-level code having to interfere.
#
# Note that the contract between publisher and subscriber (observable and
# observer) is not declared or enforced. The `Ticker` publishes a time and a
# price, and the warners receive that. But if you don't ensure that your
# contracts are correct, nothing else can warn you.
#
# ```ruby
# require "observer"
#
# class Ticker          ### Periodically fetch a stock price.
#   include Observable
#
#   def initialize(symbol)
#     @symbol = symbol
#   end
#
#   def run
#     last_price = nil
#     loop do
#       price = Price.fetch(@symbol)
#       print "Current price: #{price}\n"
#       if price != last_price
#         changed                 # notify observers
#         last_price = price
#         notify_observers(Time.now, price)
#       end
#       sleep 1
#     end
#   end
# end
#
# class Price           ### A mock class to fetch a stock price (60 - 140).
#   def self.fetch(symbol)
#     60 + rand(80)
#   end
# end
#
# class Warner          ### An abstract observer of Ticker objects.
#   def initialize(ticker, limit)
#     @limit = limit
#     ticker.add_observer(self)
#   end
# end
#
# class WarnLow < Warner
#   def update(time, price)       # callback for observer
#     if price < @limit
#       print "--- #{time.to_s}: Price below #@limit: #{price}\n"
#     end
#   end
# end
#
# class WarnHigh < Warner
#   def update(time, price)       # callback for observer
#     if price > @limit
#       print "+++ #{time.to_s}: Price above #@limit: #{price}\n"
#     end
#   end
# end
#
# ticker = Ticker.new("MSFT")
# WarnLow.new(ticker, 80)
# WarnHigh.new(ticker, 120)
# ticker.run
# ```
#
# Produces:
#
# ```
# Current price: 83
# Current price: 75
# --- Sun Jun 09 00:10:25 CDT 2002: Price below 80: 75
# Current price: 90
# Current price: 134
# +++ Sun Jun 09 00:10:25 CDT 2002: Price above 120: 134
# Current price: 134
# Current price: 112
# Current price: 79
# --- Sun Jun 09 00:10:25 CDT 2002: Price below 80: 79
# ```
module Observable
  # Add `observer` as an observer on this object. So that it will receive
  # notifications.
  #
  # `observer`
  # :   the object that will be notified of changes.
  # `func`
  # :   [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html) naming the
  #     method that will be called when this
  #     [`Observable`](https://docs.ruby-lang.org/en/2.6.0/Observable.html) has
  #     changes.
  #
  #     This method must return true for `observer.respond_to?` and will receive
  #     `*arg` when
  #     [`notify_observers`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-notify_observers)
  #     is called, where `*arg` is the value passed to
  #     [`notify_observers`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-notify_observers)
  #     by this
  #     [`Observable`](https://docs.ruby-lang.org/en/2.6.0/Observable.html)
  def add_observer(observer, func = _); end

  # [`Set`](https://docs.ruby-lang.org/en/2.6.0/Set.html) the changed state of
  # this object. Notifications will be sent only if the changed `state` is
  # `true`.
  #
  # `state`
  # :   Boolean indicating the changed state of this
  #     [`Observable`](https://docs.ruby-lang.org/en/2.6.0/Observable.html).
  def changed(state = _); end

  # Returns true if this object's state has been changed since the last
  # [`notify_observers`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-notify_observers)
  # call.
  def changed?; end

  # Return the number of observers associated with this object.
  def count_observers; end

  # Remove `observer` as an observer on this object so that it will no longer
  # receive notifications.
  #
  # `observer`
  # :   An observer of this
  #     [`Observable`](https://docs.ruby-lang.org/en/2.6.0/Observable.html)
  def delete_observer(observer); end

  # Remove all observers associated with this object.
  def delete_observers; end

  # Notify observers of a change in state **if** this object's changed state is
  # `true`.
  #
  # This will invoke the method named in
  # [`add_observer`](https://docs.ruby-lang.org/en/2.6.0/Observable.html#method-i-add_observer),
  # passing `*arg`. The changed state is then set to `false`.
  #
  # `*arg`
  # :   Any arguments to pass to the observers.
  def notify_observers(*arg); end
end
