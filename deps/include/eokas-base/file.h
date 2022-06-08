
#ifndef  _EOKAS_BASE_FILE_H_
#define  _EOKAS_BASE_FILE_H_

#include "header.h"
#include "stream.h"

_BeginNamespace(eokas)
/*
=================================================================
== FileStream
=================================================================
*/
/*
FileOpenMode
r	: readonly nocreate
w	: writeonly
a	: append
r+	: read write nocreate
w+	: read write
a+	: read append
rb, wb, ab, rb+, wb+, ab+   binary
*/
class FileStream :public Stream
{
public:
	FileStream(const String& fileName, const String& openMode);
	virtual ~FileStream();

public:
	virtual bool open() override;
	virtual void close() override;
	virtual bool isOpen() const override;
	virtual bool readable() const override;
	virtual bool writable() const override;
	virtual bool eos() const override;
	virtual size_t pos() const override;
	virtual size_t size() const override;
	virtual size_t read(void* data, size_t size) override;
	virtual size_t write(void* data, size_t size) override;
	virtual bool seek(int offset, int origin = SEEK_CUR) override;
	virtual void flush() override;

public:
	FILE* handle() const;

private:
	String mName;
	String mMode;
	FILE* mHandle;
};

/*
=================================================================
== FileInfo
=================================================================
*/
struct FileInfo
{
	String name;
	u64_t size;
	bool isFile;
	bool isHidden;
	time_t atime;
	time_t mtime;
	time_t ctime;
};

using FileList = std::list<FileInfo>;

/*
=================================================================
== File system interface
=================================================================
*/
class File
{
public:
	static bool exists(const String& path);
	static bool isFile(const String& path);
	static bool isFolder(const String& path);

	static FileList fileInfoList(const String& path);
	static StringList fileNameList(const String& path);
	static StringList folderNameList(const String& path);

	static String executingPath();
	static String absolutePath(const String& path);
	static String basePath(const String& path);
	static String fileName(const String& path);
	static String fileNameWithoutExtension(const String& path);
	static String fileExtension(const String& path);
    static String combinePath(const String& path1, const String& path2);
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_FILE_H_
