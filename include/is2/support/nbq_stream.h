//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _NBQ_STREAM_H
#define _NBQ_STREAM_H
//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include "is2/support/nbq.h"
#include <assert.h>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: nbq_stream
//! ----------------------------------------------------------------------------
class nbq_stream
{
public:
        // -------------------------------------------------
        // public types
        // -------------------------------------------------
        typedef char Ch;
        // -------------------------------------------------
        // public methods
        // -------------------------------------------------
        nbq_stream(nbq &a_nbq):
                m_nbq(a_nbq),
                m_idx(0)
        {}
        ~nbq_stream()
        {}
        // -------------------------------------------------
        // Peek
        // -------------------------------------------------
        Ch Peek() const
        {
                if(m_nbq.read_avail())
                {
                        return m_nbq.peek();
                }
                return '\0';
        }
        // -------------------------------------------------
        // Take
        // -------------------------------------------------
        Ch Take()
        {
                char l_c;
                int64_t l_s;
                l_s = m_nbq.read(&l_c, 1);
                if(l_s != 1)
                {
                        return '\0';
                }
                ++m_idx;
                return l_c;
        }
        // -------------------------------------------------
        // Tell
        // -------------------------------------------------
        size_t Tell() const
        {
                return m_idx;
        }
        Ch* PutBegin() { assert(false); return 0; }
        void Put(Ch) { assert(false); }
        void Flush() { assert(false); }
        size_t PutEnd(Ch*) { assert(false); return 0; }
private:
        // -------------------------------------------------
        // private methods
        // -------------------------------------------------
        // Disallow copy/assign
        nbq_stream& operator=(const nbq_stream &);
        nbq_stream(const nbq_stream &);
        // -------------------------------------------------
        // private members
        // -------------------------------------------------
        nbq &m_nbq;
        size_t m_idx;
};

} // ns_is2
#endif
