//! ----------------------------------------------------------------------------
//! Copyright Edgecast Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include "is2/status.h"
#include "is2/support/ndebug.h"
#include "is2/srvr/stat.h"
#include <stdio.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <math.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: TODO
//! \return:  TODO
//! \param:   n/a
//! ----------------------------------------------------------------------------
double xstat_struct::stdev() const
{
        return sqrt(var());
}
//! ----------------------------------------------------------------------------
//! \details: Update stat with new value
//! \return:  n/a
//! \param:   ao_stat stat to be updated
//! \param:   a_val value to update stat with
//! ----------------------------------------------------------------------------
void update_stat(xstat_t &ao_stat, double a_val)
{
        ao_stat.m_num++;
        ao_stat.m_sum_x += a_val;
        ao_stat.m_sum_x2 += a_val*a_val;
        if(a_val > ao_stat.m_max) ao_stat.m_max = a_val;
        if(a_val < ao_stat.m_min) ao_stat.m_min = a_val;
}
//! ----------------------------------------------------------------------------
//! \details: Add stats
//! \return:  n/a
//! \param:   ao_stat stat to be updated
//! \param:   a_from_stat stat to add
//! ----------------------------------------------------------------------------
void add_stat(xstat_t &ao_stat, const xstat_t &a_from_stat)
{
        ao_stat.m_num += a_from_stat.m_num;
        ao_stat.m_sum_x += a_from_stat.m_sum_x;
        ao_stat.m_sum_x2 += a_from_stat.m_sum_x2;
        if(a_from_stat.m_min < ao_stat.m_min)
                ao_stat.m_min = a_from_stat.m_min;
        if(a_from_stat.m_max > ao_stat.m_max)
                ao_stat.m_max = a_from_stat.m_max;
}
//! ----------------------------------------------------------------------------
//! \details: Clear stat
//! \return:  n/a
//! \param:   ao_stat stat to be cleared
//! ----------------------------------------------------------------------------
void clear_stat(xstat_t &ao_stat)
{
        ao_stat.m_sum_x = 0.0;
        ao_stat.m_sum_x2 = 0.0;
        ao_stat.m_min = 0.0;
        ao_stat.m_max = 0.0;
        ao_stat.m_num = 0;
}
//! ----------------------------------------------------------------------------
//! \details: Show stat
//! \return:  n/a
//! \param:   ao_stat stat display
//! ----------------------------------------------------------------------------
void show_stat(const xstat_t &ao_stat)
{
        ::printf("Stat: Mean: %4.2f, StdDev: %4.2f, Min: %4.2f, Max: %4.2f Num: %" PRIu64 "\n",
                 ao_stat.mean(),
                 ao_stat.stdev(),
                 ao_stat.m_min,
                 ao_stat.m_max,
                 ao_stat.m_num);
}
//! ----------------------------------------------------------------------------
//! Example...
//! ----------------------------------------------------------------------------
#if 0
int main(void)
{
    xstat_t l_s;
    update_stat(l_s, 30.0);
    update_stat(l_s, 15.0);
    update_stat(l_s, 22.0);
    update_stat(l_s, 10.0);
    update_stat(l_s, 24.0);
    show_stat(l_s);
    return 0;
}
#endif
} //namespace ns_is2 {
