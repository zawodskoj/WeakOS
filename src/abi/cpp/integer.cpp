#include <cpp/integer.h> 

// Скопипастил отсюда: https://github.com/esneider/div64/blob/master/div64.h

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor)
{
    uint64_t shift = divisor;
	uint64_t aux   = divisor;

	while ( shift < dividend && (aux <<= 1) > shift )
		shift = aux;

	for ( aux = 0; shift >= divisor; shift >>= 1 ) {
		aux <<= 1;
		if ( shift <= dividend ) {
			aux++;
			dividend -= shift;
		}
	}
	return aux;
}

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor)
{
    uint64_t shift = divisor;
    uint64_t aux   = divisor;

    while ( shift < dividend && (aux <<= 1) > shift )
        shift = aux;

    for ( aux = 0; shift >= divisor; shift >>= 1 ) {
        aux <<= 1;
        if ( shift <= dividend ) {
            aux++;
            dividend -= shift;
        }
    }
    return dividend;
}

int64_t __divdi3(int64_t dividend, int64_t divisor)
{
    bool sig = (dividend < 0) ^ (divisor < 0);
    if (dividend < 0) dividend = -dividend;
    if (divisor < 0) divisor = -divisor;
    
    uint64_t shift = divisor;
	uint64_t aux   = divisor;

	while ( shift < dividend && (aux <<= 1) > shift )
		shift = aux;

	for ( aux = 0; shift >= divisor; shift >>= 1 ) {
		aux <<= 1;
		if ( shift <= dividend ) {
			aux++;
			dividend -= shift;
		}
	}
    return sig ? -aux : aux;
}

int64_t __moddi3(int64_t dividend, int64_t divisor)
{
    bool sig = (dividend < 0) ^ (divisor < 0);
    if (dividend < 0) dividend = -dividend;
    if (divisor < 0) divisor = -divisor;
    
    uint64_t shift = divisor;
    uint64_t aux   = divisor;

    while ( shift < dividend && (aux <<= 1) > shift )
        shift = aux;

    for ( aux = 0; shift >= divisor; shift >>= 1 ) {
        aux <<= 1;
        if ( shift <= dividend ) {
            aux++;
            dividend -= shift;
        }
    }
    return sig ? -dividend : dividend;
}
