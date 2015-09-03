#ifndef WIISAVE_HPP
#define WIISAVE_HPP

#include <unordered_map>
#include <string>
#include "Athena/Global.hpp"

namespace Athena
{

class WiiFile;
class WiiBanner;
class WiiImage;
class BinaryReader;
class BinaryWriter;

/*! \class WiiSave
 *  \brief Wii data.bin container class
 *
 * Contains all relevant data for a Wii data.bin file.
 */
class WiiSave
{
public:

    /*!
     * \brief FileIterator
     */
    typedef std::unordered_map<std::string, WiiFile*>::const_iterator FileIterator;

    /*!
     * \brief WiiSave
     */
    WiiSave();
    /*!
     * \brief ~WiiSave
     */
    virtual ~WiiSave();

    /*!
     * \brief addFile
     * \param filename
     * \param file
     */
    void     addFile(WiiFile* file);
    void setRoot(WiiFile* root);
    /*!
     * \brief file
     * \param filename
     * \return
     */
    WiiFile* file(const std::string& filename);

    atUint32 fileCount() const;
    /*!
     * \brief fileList
     * \return
     */
    WiiFile* root();

    /*!
     * \brief setBanner
     * \param banner
     */
    void     setBanner(WiiBanner* banner);

    /*!
     * \brief banner
     * \return
     */
    WiiBanner* banner() const;

    std::vector<WiiFile*> allFiles() const;

protected:
private:

    WiiFile*   m_root;
    WiiBanner* m_banner;

};

} // zelda
#endif // WIISAVE_HPP
