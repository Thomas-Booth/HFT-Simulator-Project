#include "portfolio_tracker.h"



void update_portfolio(tradeType tradeDirection, double priceUsed, double volumeChange, userAccount *user) {
    if (tradeDirection == Bid) {
        user->baseCurrencyBalance += volumeChange;
    } else {
        user->quoteCurrencyBalance += (priceUsed * volumeChange); 
    }
}