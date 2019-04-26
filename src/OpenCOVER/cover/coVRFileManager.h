/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef COVR_FILE_MANAGER_H
#define COVR_FILE_MANAGER_H

/*! \file
 \brief  load files and manipulate file names

 \author (C)
         Computer Centre University of Stuttgart,
         Allmandring 30,
         D-70550 Stuttgart,
         Germany

 \date
 */

#include <util/coExport.h>
#include <list>
#include <limits.h>
#include <map>
#include <memory>
#include <osg/ref_ptr>
#include <osg/Texture2D>
#include <string>
#include <vector>
#include <vrbclient/SharedState.h>
#include <OpenVRUI/coUpdateManager.h>

namespace osg
{
class Node;
class Group;
class Texture2D;
}

namespace osgText
{
class Font;
}
namespace covise
{
class Message;
}
namespace opencover
{

namespace ui
{
class Owner;
class Group;
class FileBrowser;
};

class coTUIFileBrowserButton;
class coVRIOReader;

struct LoadedFile;

class COVEREXPORT Url
{
public:
    Url(const std::string &url);
    static Url fromFileOrUrl(const std::string &furl);
    static std::string decode(const std::string &str, bool path=false);

    std::string str() const;
    operator std::string() const;

    std::string extension() const;
    bool valid() const;
    bool isLocal() const;

    const std::string &scheme() const;
    const std::string &authority() const;
    const std::string &path() const;
    const std::string &query() const;
    const std::string &fragment() const;

private:

    bool m_valid = false;

    std::string m_scheme;
    std::string m_authority;
    bool m_haveAuthority = false;
        std::string m_userinfo;
        std::string m_host;
        std::string m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

    Url();
};


typedef struct
{
    int (*loadUrl)(const Url &url, osg::Group *parent, const char *covise_key);
    int (*loadFile)(const char *filename, osg::Group *parent, const char *covise_key);
    int (*replaceFile)(const char *filename, osg::Group *parent, const char *covise_key);
    int (*unloadFile)(const char *filename, const char *covise_key);
    const char *extension;
} FileHandler;



class COVEREXPORT coVRFileManager : public vrui::coUpdateable
{
    friend struct LoadedFile;
    static coVRFileManager *s_instance;

public:
    ~coVRFileManager();
    static coVRFileManager *instance();

    std::string findFileExt(const Url &url);

    // returns the full path for file
    const char *getName(const char *file);
    //removes sharedDataPath from filePath
    void relativePath(std::string &fileName);
    //search file locally, in sharedData and then try to remote fetch the file until a the file gets found. Return "" if no file found.
    std::string findOrGetFile(const std::string &fileName);
    // load a OSG or VRML97 or other (via plugin) file
    osg::Node *loadFile(const char *file, coTUIFileBrowserButton *fb = NULL, osg::Group *parent = NULL, const char *covise_key = "");

    // replace the last loaded Performer or VRML97 file
    osg::Node *replaceFile(const char *file, coTUIFileBrowserButton *fb = NULL, osg::Group *parent = NULL, const char *covise_key = "");

    // reload the previously loaded /*VRML*/ file
    void reloadFile();

    // unload the previously loaded file
    void unloadFile(const char *file=NULL);

    // set name of a file to store Viewpoints; if unset, this is derived from the loaded FileName
    void setViewPointFile(const std::string &viewPointFile);

    // getName of a file to store Viewpoints
    // this is derived from the loaded FileName
    std::string getViewPointFile();

    // load an icon file, looks in covise/icons/$LookAndFeel or covise/icons
    // returns NULL, if nothing found
    osg::Node *loadIcon(const char *filename);

    // loads a font
    // fontname can be NULL, which loads the default specified in the config (DroidSansFallbackFull.ttf if not in config), or "myfont.ttf"
    std::string getFontFile(const char *fontname);
    osg::ref_ptr<osgText::Font> loadFont(const char *fontname);

    // load an texture, looks in covise/icons/$LookAndFeel or covise/icons for filename.rgb
    // returns NULL, if nothing found
    osg::Texture2D *loadTexture(const char *texture);

    // tries to fopen() fileName
    // returns true if exists otherwise false
    bool fileExist(const char *fileName);

    // builds filename for icon files, looks in covise/icons/$LookAndFeel or covise/icons for filename.rgb
    const char *buildFileName(const char *);

    // register a loader, ... for a file type
    int registerFileHandler(const FileHandler *handler);
    int registerFileHandler(coVRIOReader *handler);

    // unregister a loader, ... for a file type
    int unregisterFileHandler(const FileHandler *handler);
    int unregisterFileHandler(coVRIOReader *handler);

    // get list of extensions as required by a filebrowser
    std::string getFilterList();

    // get list of extensions for saving as required by a filebrowser
    std::string getWriteFilterList();

    // get a loader for a file type, if available
    const FileHandler *getFileHandler(const char *extension);
    coVRIOReader *findIOHandler(const char *extension);

    // find a loader, load plugin if no loader available
    const FileHandler *findFileHandler(const char *extension);

    coTUIFileBrowserButton *getMatchingFileBrowserInstance(std::string keyFileName);

    bool IsDefFBSet();

    void SetDefaultFB(coTUIFileBrowserButton *fb);

    virtual bool update();
    //send a requested File to vrb
    void sendFile(covise::TokenBuffer &tb);

	///request the file from vrb -> file gets copied to tmp
	std::string remoteFetch(const std::string &filePath, int fileOwner = -1);
private:
    // Get the configured font style.
    int coLoadFontDefaultStyle();

    std::string viewPointFile;
    int m_loadCount = 0;
    std::unique_ptr<ui::Owner> m_owner;
    ui::Group *m_fileGroup = nullptr;

    typedef std::list<const FileHandler *> FileHandlerList;
    FileHandlerList fileHandlerList;

    typedef std::list<coVRIOReader *> IOReaderList;
    IOReaderList ioReaderList;

    typedef std::map<std::string, osg::ref_ptr<osg::Texture2D> > TextureMap;
    TextureMap textureList;
    std::map<std::string, coTUIFileBrowserButton *> fileFBMap;
    coTUIFileBrowserButton *mDefaultFB = nullptr;

    struct IOReadOperation
    {
        IOReadOperation()
        {
            reader = 0;
            filename = "";
            group = 0;
        }
        coVRIOReader *reader;
        std::string filename;
        osg::Group *group;
    };

    typedef std::map<std::string, std::list<IOReadOperation> > ReadOperations;
    ReadOperations readOperations;

    coVRFileManager();
    LoadedFile *m_lastFile = nullptr;
    LoadedFile *m_loadingFile = nullptr;
    std::map<std::string, LoadedFile *> m_files;
	///map of fileowners(client id) and file paths
	typedef std::map<std::string, int> fileOwnerMap;
    vrb::SharedState<fileOwnerMap> m_sharedFiles;
    void loadPartnerFiles();
    struct Compare {
        bool operator()(const std::string& first, const std::string& second) {
            return first.size() > second.size();
        }
    };
    ///returns the full path of the symbolic link that points to the shared data
    ///the link should be in COVISE_PATH/.. and named sharedData
    void getSharedDataPath();
    std::string m_sharedDataPath;
    void convertBackslash(std::string &path);
    std::string getFileName(const std::string & fileName);
    ///if fileName contains m_sharedDataPath, cut it out
    std::string cutFileName(const std::string & fileName);
    ///rosolves links in the file path from filename, cutName(fileName) and removs non-aphanumeric characters
    std::string reduceToAlphanumeric(const std::string & str);
	///converts the path to a shared path
	std::string getPathIdentifier(const std::string& path);
	///writes content into a file unter tmp/OpenCOVER/fileName. Returns the path to the file or "" on failure
	std::string writeTmpFile(const std::string& fileName, const char* content, int size);
	///compares the filePaths of m_sharedFiels wit filePath and returns the best matching fileOwner
	int guessFileOwner(const std::string& filePath);
	std::vector<covise::Message*> m_sendFileMessages;
};
}
#endif
