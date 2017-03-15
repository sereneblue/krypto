/**
 * @file api.h
 * @author sereneblue
 * @date March 11 2017
 * @brief api header
 */

#ifndef api__h
#define api__h

/**
 * @brief Represents currency data
 */

typedef struct currency{
	double price;
	double high;
	double low;
	double market_cap;
	double change_price;
	double change_amt;
	double volume;
	char *symbol;
} currency;

/**
 * @brief Represents a cryptocurrency
 */

typedef struct cryptocurrency{
	char *full;
	char *sym;
	currency currency[5];
	double circulating;
} cryptocurrency;

extern cryptocurrency lst[5];

#endif