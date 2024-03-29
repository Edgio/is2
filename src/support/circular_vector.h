//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _CIRCULAR_VECTOR_H
#define _CIRCULAR_VECTOR_H

//! ----------------------------------------------------------------------------
//! Includes
//! ----------------------------------------------------------------------------
#include "ndebug.h"
#include <vector>

//! ----------------------------------------------------------------------------
//! Constants
//! ----------------------------------------------------------------------------
#define CIRCULAR_VECTOR_DEFAULT_MAX_SIZE 1024
#define CIRCULAR_VECTOR_DEFAULT_NO_MAX -1

//! ----------------------------------------------------------------------------
//! \details: circular_vector
//! ----------------------------------------------------------------------------
template<class _Tp>
class circular_vector
{
public:
        circular_vector(int32_t a_max_size = CIRCULAR_VECTOR_DEFAULT_MAX_SIZE) :
                m_vector(),
                m_max_size(a_max_size),
                m_cur_index(-1),
                m_start_index(0)
        {

        }
        const _Tp &get_obj(uint32_t a_index) const
        {
                // Mind wrap around.
                if(a_index >= m_vector.size())
                {
                        a_index = a_index - m_vector.size();
                }
                return m_vector[a_index];
        }
        void add_obj(const _Tp&a_obj)
        {
                ++m_cur_index;
                if(CIRCULAR_VECTOR_DEFAULT_NO_MAX == m_max_size)
                {
                        m_vector.push_back(a_obj);
                        return;
                }

                if(m_cur_index >= m_max_size)
                {
                        m_cur_index = 0;
                }
                if(m_vector.size() < ((uint32_t)m_cur_index + 1))
                {
                        //TRC_TRACE_PRINT("ADD_OBJ: %d\n", (int)m_vector.size());
                        m_vector.push_back(a_obj);
                }
                else
                {
                        //TRC_TRACE_PRINT("ADD_OBJ: %d\n", m_cur_index);
                        m_vector[m_cur_index] = a_obj;

                        // Overwriting -need to update where last start is
                        m_start_index = m_cur_index + 1;
                        if(m_start_index >= m_max_size)
                        {
                                m_start_index = 0;
                        }

                }
        }
        uint32_t get_cur_index(void) const { return m_cur_index;}
        uint32_t get_start_index(void) const { return m_start_index;}
        uint32_t get_distance_to_cur_index(uint32_t a_from_index) const
        {
                //TRC_TRACE_PRINT("m_cur_index: %d -- a_from_index = %u\n", m_cur_index, a_from_index);
                if(m_cur_index < 0)
                        return 0;

                if((uint32_t)m_cur_index < a_from_index)
                {
                        return m_vector.size() - a_from_index + m_cur_index;
                }
                else
                {
                        return m_cur_index - a_from_index;
                }
        }
private:
        typedef std::vector<_Tp> c_vector_t;
        c_vector_t m_vector;
        int32_t m_max_size;
        int32_t m_cur_index;
        int32_t m_start_index;
};

#endif
