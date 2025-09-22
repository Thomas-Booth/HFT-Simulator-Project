#include "portfolio_tracker.h"

// Change values in the user's balances -- Project currently used for forex currencies
void update_portfolio(tradeType tradeDirection, double priceUsed, double volumeChange, userAccount *user) {
    if (tradeDirection == Bid) {
        // Buying base currency, simply add the new volume
        user->baseCurrencyBalance += volumeChange;
        // Cost of this bid would be the bidPrice * the quantity
        user->quoteCurrencyBalance -= (priceUsed * volumeChange);
    } else {
        // Selling would grant quantity * sellPrice amount of quote currency
        user->quoteCurrencyBalance += (priceUsed * volumeChange);
        // Simply remove the volume of base currency we sold
        user->baseCurrencyBalance -= volumeChange; 
    }
}