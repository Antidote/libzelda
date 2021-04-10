#include "athena/SkywardSwordFileWriter.hpp"
#include "athena/SkywardSwordFile.hpp"
#include "athena/SkywardSwordQuest.hpp"

namespace athena::io {

SkywardSwordFileWriter::SkywardSwordFileWriter(atUint8* data, atUint64 len) : MemoryCopyWriter(data, len) {
  setEndian(Endian::Big);
}

SkywardSwordFileWriter::SkywardSwordFileWriter(const std::string& filename) : MemoryCopyWriter(filename) {
  setEndian(Endian::Big);
}

void SkywardSwordFileWriter::write(SkywardSwordFile* file) {
  if (!file) {
    atError(fmt("file cannot be NULL"));
    return;
  }

  atUint32 magic = (file->region() == Region::NTSC
                        ? SkywardSwordFile::USMagic
                        : (file->region() == Region::NTSCJ ? SkywardSwordFile::JAMagic : SkywardSwordFile::EUMagic));

  writeUint32(magic);
  seek(0x1C, SeekOrigin::Begin);
  writeUint32(0x1D);

  std::vector<SkywardSwordQuest*> quests = file->questList();
  int i = 0;

  for (SkywardSwordQuest* q : quests) {
    if (q->length() != 0x53C0) {
      atError(fmt("q->data() not 0x53C0 bytes in length"));
      return;
    }

    // Update the checksums
    q->fixChecksums();
    // Write the save data
    writeUBytes(q->data(), q->length());
    atUint64 pos = position();
    // Write the slots skip data
    seek(0xFB60 + (i * 0x24), SeekOrigin::Begin);
    writeUBytes(q->skipData(), 0x24);
    seek(pos, SeekOrigin::Begin);
    i++;
  }

  // write those padding bytes
  seek(0xFBE0, SeekOrigin::Begin);
  save();
}

} // namespace athena::io
