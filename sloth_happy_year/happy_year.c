#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

/**
 * @brief Get the len/size of a given number
 */
size_t size(uint32_t year)
{
    size_t idx = 0;
    uint32_t supremum = 1;
    while( supremum < year )
    {
        ++idx;
        supremum *= 10;
    }

    return idx;
}

/**
 * Get the nth position in a number
 */
uint32_t nth_number(uint32_t year, size_t len_of_year, size_t idx)
{
    const uint32_t supremum = pow(10, len_of_year - idx);
    const uint32_t div = supremum / 10;

    return (year % supremum) / div;
}

/**
 * @brief Look into the year, get the indexes of equal numbers.
 *
 * Write the index into @see i,j
 */
bool find_equal_values(uint32_t year, size_t len_of_year, size_t *i, size_t *j)
{
    for(size_t iidx = 0; iidx < len_of_year; ++iidx)
    {
        const uint32_t value_i = nth_number(year, len_of_year, iidx);

        for(size_t jidx = iidx + 1; jidx < len_of_year; ++jidx)
        {
            const uint32_t value_j = nth_number(year, len_of_year, jidx);

            if(value_i == value_j)
            {
                *i = iidx;
                *j = jidx;
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Get this or the next happy year.
 *
 * This year is returned if the happy year condition is already fulfilled.
 */
uint32_t this_or_next_happy_year(uint32_t year)
{
    assert(year < 9999 && "must be smaller 10000");

    size_t len_of_year = size(year);

    size_t i = 0,j = 0;
    while(find_equal_values(year, len_of_year, &i, &j))
    {
        year += pow(10, len_of_year - j - 1);
    }

    return year;
}

/**
 * @brief Get the 'next' happy year
 *
 * If the current year IS the happy year, the year is
 * incremented and run anew.
 */
uint32_t next_happy_year(uint32_t year)
{
    const uint32_t new_year = this_or_next_happy_year(year);

    if(new_year == year)
    {
        return this_or_next_happy_year(year + 1);
    }

    return new_year;
}

/**
 * @brief main with year examples from task
 */
int main(int argc, char **argv)
{
    uint32_t nums[] = {1990, 2017, 2021};

    for (const uint32_t *begin = nums; begin < nums + sizeof(nums)/sizeof(uint32_t); ++begin)
    {
        printf("next happy year after %d := %d\n", *begin, next_happy_year(*begin));
    }

    return 0;
}
