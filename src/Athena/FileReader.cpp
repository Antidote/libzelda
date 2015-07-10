#include "Athena/FileReader.hpp"
#include "Athena/FileNotFoundException.hpp"
#include "Athena/InvalidDataException.hpp"
#include "Athena/InvalidOperationException.hpp"
#include "Athena/IOException.hpp"

#if _WIN32
#include "win32_largefilewrapper.h"
#elif __APPLE__
#include "osx_largefilewrapper.h"
#endif

namespace Athena
{
namespace io
{
FileReader::FileReader(const std::string& filename, atUint32 cacheSize)
    : m_filename(filename),
      m_fileHandle(nullptr),
      m_cacheData(nullptr),
      m_offset(0)
{
    setCacheSize(cacheSize);
    open();
}

FileReader::~FileReader()
{
    if (isOpen())
        close();
}

void FileReader::open()
{
    m_fileHandle = fopen(m_filename.c_str(), "rb");

    if (!m_fileHandle)
        THROW_FILE_NOT_FOUND_EXCEPTION(m_filename);

    // ensure we're at the beginning of the file
    rewind(m_fileHandle);
}

void FileReader::close()
{
    if (!m_fileHandle)
        THROW_INVALID_OPERATION_EXCEPTION("Cannot close an unopened stream");

    fclose(m_fileHandle);
    m_fileHandle = NULL;
    return;
}

void FileReader::seek(atInt64 pos, SeekOrigin origin)
{
    // check block position
    if (m_blockSize > 0)
    {
        atUint64 oldOff = m_offset;
        switch(origin)
        {
            case SeekOrigin::Begin:
                m_offset = pos;
                break;
            case SeekOrigin::Current:
                m_offset += pos;
                break;
            case SeekOrigin::End:
                m_offset = length() - pos;
                break;
        }
        if (m_offset > length())
        {
            oldOff = m_offset;
            THROW_INVALID_OPERATION_EXCEPTION("Unable to seek in file");
        }

        size_t block = m_offset / m_blockSize;
        if (block != m_curBlock)
        {
            fseeko64(m_fileHandle, block * m_blockSize, SEEK_SET);
            fread(m_cacheData.get(), 1, m_blockSize, m_fileHandle);
            m_curBlock = block;
        }
    }
    else if (fseeko64(m_fileHandle, pos, (int)origin) != 0)
        THROW_INVALID_OPERATION_EXCEPTION("Unable to seek in file");
}

atUint64 FileReader::position() const
{
    if (!isOpen())
        THROW_INVALID_OPERATION_EXCEPTION_RETURN(0, "File not open");

    if (m_blockSize > 0)
        return m_offset;
    else
        return ftello64(m_fileHandle);
}

atUint64 FileReader::length() const
{
    if (!isOpen())
        THROW_INVALID_OPERATION_EXCEPTION_RETURN(0, "File not open");

    return utility::fileSize(m_filename);
}

atUint64 FileReader::readUBytesToBuf(void* buf, atUint64 len)
{
    if (!isOpen())
        THROW_INVALID_OPERATION_EXCEPTION_RETURN(0, "File not open for reading");

    if (m_blockSize <= 0)
        return fread(buf, 1, len, m_fileHandle);
    else
    {
        size_t block = m_offset / m_blockSize;
        atUint64 cacheOffset = m_offset % m_blockSize;
        atUint64 cacheSize;
        atUint64 rem = len;
        atUint8* dst = (atUint8*)buf;

        while (rem)
        {
            if (block != m_curBlock)
            {
                fseeko64(m_fileHandle, block * m_blockSize, SEEK_SET);
                fread(m_cacheData.get(), 1, m_blockSize, m_fileHandle);
                m_curBlock = block;
            }

            cacheSize = rem;
            if (cacheSize + cacheOffset > m_blockSize)
                cacheSize = m_blockSize - cacheOffset;

            memcpy(dst, m_cacheData.get() + cacheOffset, cacheSize);
            dst += cacheSize;
            rem -= cacheSize;
            cacheOffset = 0;
            ++block;
        }
        m_offset += len;
        return dst - (atUint8*)buf;
    }
}

void FileReader::setCacheSize(const atUint32 blockSize)
{
    m_blockSize = blockSize;
    m_curBlock = -1;
    if (m_blockSize > 0)
        m_cacheData.reset(new atUint8[m_blockSize]);
}

} // io
} // Athena
