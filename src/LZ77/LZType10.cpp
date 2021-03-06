#include "LZ77/LZType10.hpp"

#include <cstddef>
#include <cstring>
#include <memory>

#include "LZ77/LZLookupTable.hpp"

#include <athena/MemoryWriter.hpp>

LZType10::LZType10(atInt32 MinimumOffset, atInt32 SlidingWindow, atInt32 MinimumMatch, atInt32 BlockSize)
: LZBase(MinimumOffset, SlidingWindow, MinimumMatch, BlockSize) {
  // ReadAheadBuffer is normalize between (minumum match) and(minimum match + 15) so that matches fit within
  // 4-bits.
  m_readAheadBuffer = m_minMatch + 0xF;
}

atUint32 LZType10::compress(const atUint8* src, atUint8** dstBuf, atUint32 srcLength) {
  atUint32 encodeSize = (srcLength << 8) | (0x10);
  encodeSize = athena::utility::LittleUint32(encodeSize); // File size needs to be written as little endian always

  athena::io::MemoryCopyWriter outbuf("tmp");
  outbuf.writeUint32(encodeSize);

  const atUint8* ptrStart = src;
  const atUint8* ptrEnd = src + srcLength;

  // At most their will be two bytes written if the bytes can be compressed. So if all bytes in the block can be
  // compressed it would take blockSize*2 bytes
  auto compressedBytes = std::unique_ptr<atUint8[]>(new atUint8[m_blockSize * 2]); // Holds the compressed bytes yet to be written

  while (ptrStart < ptrEnd) {
    atUint8 blockLen = 0;
    // In Binary represents 1 if byte is compressed or 0 if not compressed
    // For example 01001000 means that the second and fifth byte in the blockSize from the left is compressed
    atUint8* ptrBytes = compressedBytes.get();

    for (atInt32 i = 0; i < m_blockSize; i++) {
      // length_offset searchResult=Search(ptrStart, filedata, ptrEnd);
      const LZLengthOffset searchResult = m_lookupTable.search(ptrStart, src, ptrEnd);

      // If the number of bytes to be compressed is at least the size of the Minimum match
      if (searchResult.length >= static_cast<atUint32>(m_minMatch)) {
        // Gotta swap the bytes since system is wii is big endian and most computers are little endian
        atUint16 lenOff = (((searchResult.length - m_minMatch) & 0xF) << 12) | ((searchResult.offset - 1) & 0xFFF);
        athena::utility::BigUint16(lenOff);

        memcpy(ptrBytes, &lenOff, sizeof(atUint16));

        ptrBytes += sizeof(atUint16);

        ptrStart += searchResult.length;

        blockLen |= (1 << (7 - i));
        // Stores which of the next 8 bytes is compressed
        // bit 1 for compress and bit 0 for not compressed
      } else {
        *ptrBytes++ = *ptrStart++;
      }
    }

    outbuf.writeByte(blockLen);
    outbuf.writeUBytes(compressedBytes.get(), static_cast<atUint64>(ptrBytes - compressedBytes.get()));
  }

  // Add zeros until the file is a multiple of 4
  while ((outbuf.position() % 4) != 0) {
    outbuf.writeByte(0);
  }

  *dstBuf = outbuf.data();
  outbuf.save();
  return static_cast<atUint32>(outbuf.length());
}

atUint32 LZType10::decompress(const atUint8* src, atUint8** dst, atUint32 srcLength) {
  if (*src != 0x10) {
    return 0;
  }

  // Size of data when it is uncompressed
  atUint32 uncompressedSize;
  std::memcpy(&uncompressedSize, src, sizeof(uncompressedSize));

  // The compressed file has the filesize encoded in little endian
  athena::utility::LittleUint32(uncompressedSize);

  // first byte is the encode flag
  uncompressedSize = uncompressedSize >> 8;

  auto uncompressedData = std::unique_ptr<atUint8[]>(new atUint8[uncompressedSize]);
  atUint8* outputPtr = uncompressedData.get();
  atUint8* outputEndPtr = uncompressedData.get() + uncompressedSize;
  const atUint8* inputPtr = src + 4;
  const atUint8* inputEndPtr = src + srcLength;

  while (inputPtr < inputEndPtr && outputPtr < outputEndPtr) {
    const atUint8 isCompressed = *inputPtr++;

    for (atUint32 i = 0; i < static_cast<atUint32>(m_blockSize); i++) {
      // Checks to see if the next byte is compressed by looking
      // at its binary representation - E.g 10010000
      // This says that the first extracted byte and the four extracted byte is compressed
      if ((isCompressed >> (7 - i)) & 0x1) {
        atUint16 lenOff;
        memcpy(&lenOff, inputPtr, sizeof(atUint16));
        athena::utility::BigUint16(lenOff);
        inputPtr += sizeof(atUint16); // Move forward two bytes
        // length offset pair has been decoded.
        LZLengthOffset decoding;
        decoding.length = (lenOff >> 12) + m_minMatch;
        decoding.offset = static_cast<atUint16>((lenOff & 0xFFF) + 1);

        if ((outputPtr - decoding.offset) < uncompressedData.get()) {
          // If the offset to look for uncompressed is passed the current uncompresed data then the data is not
          // compressed
          dst = nullptr;
          return 0;
        }

        for (size_t j = 0; j < decoding.length; ++j) {
          outputPtr[j] = (outputPtr - decoding.offset)[j];
        }

        outputPtr += decoding.length;
      } else {
        *outputPtr++ = *inputPtr++;
      }

      if (!(inputPtr < inputEndPtr && outputPtr < outputEndPtr)) {
        break;
      }
    }
  }

  *dst = uncompressedData.release();

  return uncompressedSize;
}
