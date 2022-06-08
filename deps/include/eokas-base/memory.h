
#ifndef  _EOKAS_BASE_MEMORY_H_
#define  _EOKAS_BASE_MEMORY_H_

#include "header.h"
#include "stream.h"

_BeginNamespace(eokas)

/*
============================================================================================
==== MemoryUtility
============================================================================================
*/
class MemoryUtility
{
public:
	static void* alloc(size_t size);
	static void* alloc(size_t size, void* data);
	static void* alloc(size_t size, void* data, size_t leng);
	static void* realloc(void* ptr, size_t size);
	static void free(void* ptr);
	static void clear(void* ptr, size_t size, u8_t value);	
	static void copy(void* dst, void* src, size_t size);
	static int compare(void* ptr1, void* ptr2, size_t size);

	/// 1:r, 2:w, 3:rw, 4:x, 5:rx, 6:wx, 7:rwx
	static void* alloc_v(size_t size, u32_t prot);
	static void free_v(void* ptr, size_t size);
	static u32_t prot_v(void* ptr, size_t size, u32_t proto);
};

/*
============================================================================================
==== MemoryBuffer
============================================================================================
*/
class MemoryBuffer
{
public:
	MemoryBuffer();
	MemoryBuffer(MemoryBuffer&& temp);
	MemoryBuffer(const MemoryBuffer& other);
	MemoryBuffer(size_t size);
	MemoryBuffer(size_t size, void* data, size_t leng);
	MemoryBuffer(void* data, size_t size, bool newm = true);
	~MemoryBuffer();

public:
	MemoryBuffer& operator=(MemoryBuffer&& temp);
	MemoryBuffer& operator=(const MemoryBuffer& other);

public:
	void* const data() const;
	size_t size() const;
	void fill(void* data);
	void fill(void* data, size_t size);
	void clear();
	bool expand(size_t size);

private:
	void* mData;
	size_t mSize;
	bool mIsNewm;
};

/*
============================================================================================
==== MemoryStream
============================================================================================
*/
class MemoryStream :public Stream
{
public:
	MemoryStream();
	MemoryStream(MemoryStream&& temp);
	MemoryStream(const MemoryStream& other);
	MemoryStream(MemoryBuffer* memoryBuffer);
	MemoryStream(void* data, size_t size);
	virtual ~MemoryStream();

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
	virtual bool seek(int offset, int origin) override; // 0:beg, 1:cur, 2:end
	virtual void flush() override;

public:
	MemoryStream& operator=(MemoryStream&& temp);
	MemoryStream& operator=(const MemoryStream& other);

    void* data() const;

private:
	MemoryBuffer* mBuffer;
	bool mIsNewm;
	bool mIsOpen;
	size_t mPos;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_MEMORY_H_
