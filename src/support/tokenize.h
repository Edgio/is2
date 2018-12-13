//: ----------------------------------------------------------------------------
//: Copyright (C) 2018 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    tokenize.h
//: \details: TODO
//: \author:  David Andrews
//: \date:    03/04/2015
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
#include <string>
namespace ns_is2 {
//: ----------------------------------------------------------------------------
//: \details: Tokenize the provided string based on a number of delimiters
//:           from: http://stackoverflow.com/questions/236129/split-a-string-in-c
//: \return  The number of tokens added
//: \param   a_str       The string to tokenize
//: \param   ao_offsets  The offset of the start of each string provided in the
//:                      return container
//: ----------------------------------------------------------------------------
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
