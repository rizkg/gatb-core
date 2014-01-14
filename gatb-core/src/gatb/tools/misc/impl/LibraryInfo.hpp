/*****************************************************************************
 *   GATB : Genome Assembly Tool Box
 *   Copyright (C) 2014  R.Chikhi, G.Rizk, E.Drezen
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

/** \file LibraryInfo.hpp
 *  \date 01/03/2013
 *  \author edrezen
 *  \brief
 */

#ifndef _GATB_CORE_TOOLS_MISC_IMPL_LIBRARY_INFO_HPP_
#define _GATB_CORE_TOOLS_MISC_IMPL_LIBRARY_INFO_HPP_

/********************************************************************************/

#include <gatb/system/impl/System.hpp>
#include <gatb/tools/misc/impl/Property.hpp>

/********************************************************************************/
namespace gatb      {
namespace core      {
namespace tools     {
namespace misc      {
namespace impl      {
/********************************************************************************/

/** \brief Framework class for implementing tools (ie. binary tools).
 */
class LibraryInfo
{
public:

    static LibraryInfo& singleton()  { static LibraryInfo instance; return instance; }

    Properties& getInfo() { return _props; }

private:

    LibraryInfo ()
    {
        _props.add (0, "gatb-core-library");
        _props.add (1, "version",        "%s", system::impl::System::info().getVersion().c_str());
        _props.add (1, "build_date",     "%s", system::impl::System::info().getBuildDate().c_str());
        _props.add (1, "build_system",   "%s", system::impl::System::info().getBuildSystem().c_str());
        _props.add (1, "build_compiler", "%s", system::impl::System::info().getBuildCompiler().c_str());
        _props.add (1, "build_options",  "%s", system::impl::System::info().getBuildOptions().c_str());
    }

    Properties _props;
};

/********************************************************************************/
} } } } } /* end of namespaces. */
/********************************************************************************/

#endif /* _GATB_CORE_TOOLS_MISC_IMPL_LIBRARY_INFO_HPP_ */
