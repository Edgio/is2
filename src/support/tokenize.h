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
#include <string>
namespace ns_is2 {
//! ----------------------------------------------------------------------------
//! \details: Tokenize the provided string based on a number of delimiters
//!           from: http://stackoverflow.com/questions/236129/split-a-string-in-c
//! \return  The number of tokens added
//! \param   a_str       The string to tokenize
//! \param   ao_offsets  The offset of the start of each string provided in the
//!                      return container
//! ----------------------------------------------------------------------------
template <class _Container_Tp>
int tokenize(const std::string& str,
             const std::string& delimiters = " ",
             _Container_Tp* ao_tokens = nullptr,
             std::vector <int>* ao_offsets = nullptr,
             bool trimEmpty = false)
{
        int retval = 0;
        if(ao_offsets == nullptr &&
           ao_tokens == nullptr)
        {
                // not adding anything as nothing to add to
                return 0;
        }
        std::string::size_type pos, lastPos = 0;
        typedef typename _Container_Tp::value_type value_type;
        typedef typename _Container_Tp::size_type size_type;
        while(true)
        {
                pos = str.find_first_of(delimiters, lastPos);
                if(pos == std::string::npos)
                {
                        pos = str.length();
                        if(pos != lastPos || !trimEmpty)
                        {
                                if(ao_tokens != nullptr)
                                {
                                        ao_tokens->push_back(value_type(str.data()+lastPos,
                                                                       (size_type)pos-lastPos));
                                }
                                if(ao_offsets != nullptr)
                                {
                                        ao_offsets->push_back(lastPos);
                                }
                                ++retval;
                        }
                        break;
                }
                else
                {
                        if(pos != lastPos || !trimEmpty)
                        {
                                if(ao_tokens != nullptr)
                                {
                                        ao_tokens->push_back(value_type(str.data()+lastPos,
                                                                       (size_type)pos-lastPos));
                                }
                                if(ao_offsets != nullptr)
                                {
                                        ao_offsets->push_back(lastPos);
                                }
                                ++retval;
                        }
                }
                lastPos = pos + 1;
        }
        return retval;
}
} // end namespace ns_is2 {
