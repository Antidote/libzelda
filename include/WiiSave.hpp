// This file is part of libZelda.
//
// libZelda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libZelda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libZelda.  If not, see <http://www.gnu.org/licenses/>
#ifndef __WII__SAVE_HPP__
#define __WII__SAVE_HPP__

#include <unordered_map>
#include <string>
#include <Types.hpp>

class WiiFile;
class WiiBanner;
class WiiImage;
class BinaryReader;
class BinaryWriter;

class WiiSave
{
public:

    typedef std::unordered_map<std::string, WiiFile*>::const_iterator FileIterator;
    WiiSave();
    virtual ~WiiSave();

    void     addFile(const std::string& filename, WiiFile* file);
    WiiFile* file(const std::string& filename) const;
    std::unordered_map<std::string, WiiFile*>& fileList();

    void     setBanner(WiiBanner* banner);
    WiiBanner* banner() const;

protected:
private:

    std::unordered_map<std::string, WiiFile*> m_files;
    WiiBanner* m_banner;

};

#endif // __WII__SAVE_HPP__
