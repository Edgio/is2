//! ----------------------------------------------------------------------------
//! Copyright Edgio Inc.
//!
//! \file:    TODO
//! \details: TODO
//!
//! Licensed under the terms of the Apache 2.0 open source license.
//! Please refer to the LICENSE file in the project root for the terms.
//! ----------------------------------------------------------------------------
#ifndef _MIME_TYPES_H
#define _MIME_TYPES_H
//! ----------------------------------------------------------------------------
//! includes
//! ----------------------------------------------------------------------------
#include <string>
#include <map>
//! ----------------------------------------------------------------------------
//! Macros
//! ----------------------------------------------------------------------------
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! mime types
//! ----------------------------------------------------------------------------
class mime_types
{
public:
        // -------------------------------------------------
        // Types
        // -------------------------------------------------
        typedef std::map<std::string, std::string> ext_type_map_t;
        // -------------------------------------------------
        // ext->type pair
        // -------------------------------------------------
        struct T
        {
                const char* m_ext;
                const char* m_content_type;
                operator ext_type_map_t::value_type() const
                {
                        return std::pair<std::string, std::string>(m_ext, m_content_type);
                }
        };
        // -------------------------------------------------
        // Private class members
        // -------------------------------------------------
        static const T S_EXT_TYPE_PAIRS[];
        static const ext_type_map_t S_EXT_TYPE_MAP;
};
//! ----------------------------------------------------------------------------
//! Generated file extensions -> mime types associations
//! ----------------------------------------------------------------------------
const mime_types::T mime_types::S_EXT_TYPE_PAIRS[] =
{
#include "_gen_mime_types.h"
};
//! ----------------------------------------------------------------------------
//! Map
//! ----------------------------------------------------------------------------
const mime_types::ext_type_map_t mime_types::S_EXT_TYPE_MAP(S_EXT_TYPE_PAIRS,
                                                            S_EXT_TYPE_PAIRS +
                                                                    ARRAY_SIZE(mime_types::S_EXT_TYPE_PAIRS));
} // ns_is2
#endif
