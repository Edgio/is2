//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    time_util.cc
//: \details: TODO
//: \author:  Reed P. Morrison
//: \date:    01/06/2018
//:
//:   Licensed under the Apache License, Version 2.0 (the "License");
//:   you may not use this file except in compliance with the License.
//:   You may obtain a copy of the License at
//:
//:       http://www.apache.org/licenses/LICENSE-2.0
//:
//:   Unless required by applicable law or agreed to in writing, software
//:   distributed under the License is distributed on an "AS IS" BASIS,
//:   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//:   See the License for the specific language governing permissions and
//:   limitations under the License.
//:
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include <unistd.h>
#include <time.h>
#include "date_util.h"
// Mach time support clock_get_time
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
//: ----------------------------------------------------------------------------
//: Constants
//: ----------------------------------------------------------------------------
#define MAX_TIMER_RESOLUTION_US 1000000
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: global static
//: ----------------------------------------------------------------------------
uint64_t g_cyles_us               = 0;
uint64_t g_last_s                 = 0;
// Date cache...
uint64_t g_last_date_str_s_rdtsc  = 0;
uint64_t g_last_date_str_s        = 0;
char g_last_date_str[128];

//: ----------------------------------------------------------------------------
//: \details: Get the rdtsc value
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
static __inline__ uint64_t get_rdtsc64()
{
        uint32_t l_lo;
        uint32_t l_hi;
        // We cannot use "=A", since this would use %rax on x86_64
        __asm__ __volatile__ ("rdtsc" : "=a" (l_lo), "=d" (l_hi));
        // output registers
        return (uint64_t) l_hi << 32 | l_lo;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
static inline bool _use_cached_time(uint64_t &a_last_rdtsc)
{
        if(!g_cyles_us)
        {
                uint64_t l_start = get_rdtsc64();
                usleep(100000);
                g_cyles_us = (get_rdtsc64()-l_start)/100000;
        }
        if((get_rdtsc64() - a_last_rdtsc) < MAX_TIMER_RESOLUTION_US*g_cyles_us)
        {
                return true;
        }
        a_last_rdtsc = get_rdtsc64();
        return false;
}
//: ----------------------------------------------------------------------------
//: \details: TODO
//: \return:  TODO
//: \param:   TODO
//: ----------------------------------------------------------------------------
const char *get_date_str(void)
{
        if(_use_cached_time(g_last_date_str_s_rdtsc) && g_last_s)
        {
                return g_last_date_str;
        }
        time_t l_now = time(0);
        struct tm l_tm = *gmtime(&l_now);
        strftime(g_last_date_str, sizeof g_last_date_str, "%d/%b/%Y:%H:%M:%S %z", &l_tm);
        return g_last_date_str;
}
}
