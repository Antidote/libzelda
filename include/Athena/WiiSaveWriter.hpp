#ifndef ATHENA_NO_SAVES
// This file is part of libAthena.
//
// libAthena is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libAthena is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libAthena.  If not, see <http://www.gnu.org/licenses/>

#ifndef WIISAVEWRITER_HPP
#define WIISAVEWRITER_HPP

#include "Athena/MemoryWriter.hpp"

namespace Athena
{
class WiiSave;
class WiiBanner;
class WiiFile;
class WiiImage;

namespace io
{

/*! \class WiiSaveWriter
 *  \brief Wii data.bin writer class
 *
 *  A Class for writing binary data to a wii data.bin file,
 *  all work is done using a memory buffer, and not written directly to the disk.
 *  \sa BinaryReader
 */
class WiiSaveWriter : protected MemoryWriter
{
    MEMORYWRITER_BASE();
public:
    /*! \brief This constructor creates an instance from a file on disk.
     *
     * \param filename The file to create the stream from
     */
    WiiSaveWriter(const std::string&);

    /*!
     * \brief writeSave
     * \param save
     * \param macAddress
     * \param ngId
     * \param ngPriv
     * \param ngSig
     * \param ngKeyId
     * \param filepath
     * \return
     */
    bool writeSave(WiiSave* save, atUint8* macAddress, atUint32 ngId, atUint8* ngPriv, atUint8* ngSig, atUint32 ngKeyId, const std::string& filepath = "");

private:
    void writeBanner(WiiBanner* banner);
    atUint32 writeFile(WiiFile* file);
    void writeImage(WiiImage* image);
    void writeCerts(atUint32 filesSize, atUint32 ngId, atUint8* ngPriv, atUint8* ngSig, atUint32 ngKeyId);
};

} // io
} // zelda
#endif // WIISAVEWRITER_HPP
#endif // ATHENA_NO_SAVES