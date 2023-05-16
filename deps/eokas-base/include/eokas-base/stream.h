
#ifndef _EOKAS_BASE_STREAM_H_
#define _EOKAS_BASE_STREAM_H_

#include "./header.h"
#include "./string.h"

_BeginNamespace(eokas)

class Stream :public Interface
{
public:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual bool readable() const = 0;
    virtual bool writable() const = 0;
    virtual bool eos() const = 0;
    virtual size_t pos() const = 0;
    virtual size_t size() const = 0;
    virtual size_t read(void* data, size_t size) = 0;
    virtual size_t write(void* data, size_t size) = 0;
    virtual bool seek(int offset, int origin) = 0; // 0:beg, 1:cur, 2:end
    virtual void flush() = 0;

public:
    void read(Stream& stream);
    void write(Stream& stream);
};


class DataStream :public Stream
{
public:
    DataStream(Stream& target);

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
    Stream& target() const;
    void bind(Stream& target);

private:
    Stream* mTarget;
};


class BinaryStream :public DataStream
{
public:
    BinaryStream(Stream& target)
        : DataStream(target)
    {}

    template<typename T>
    inline bool read(T& value);
	
    template <typename T>
    inline bool write(const T& value);
};

template<typename T>
inline bool BinaryStream::read(T& value)
{
    Stream& base = *this;
    size_t size = sizeof(T);
    size_t rlen = base.read((void*)&value, size);
    return rlen == size;
}
template<>
inline bool BinaryStream::read<String>(String& value)
{
    Stream& base = *this;
    u16_t size = 0;
    if (!this->read(size))
        return false;
    value = String('\0', (size_t)size);
    size_t rlen = base.read((void*)value.cstr(), size);
    return rlen == size;
}

template <typename T>
inline bool BinaryStream::write(const T& value)
{
    Stream& base = *this;
    size_t size = sizeof(T);
    size_t wlen = base.write((void*)&value, size);
    return wlen == size;
}
template <>
inline bool BinaryStream::write<String>(const String& value)
{
    Stream& base = *this;
    u16_t size = (u16_t)value.length();
    if (!this->write(size))
        return false;
    size_t wlen = base.write((void*)value.cstr(), size);
    return wlen == size;
}

class TextStream :public DataStream
{
public:
    TextStream(Stream& target)
        : DataStream(target)
    {}

    bool read(char& value)
    {
        Stream& base = *this;
        size_t rlen = base.read((void*)&value, 1);
        return rlen == 1;
    }

    bool read(String& value)
    {
        Stream& base = *this;
        char c = '\0';
        if (!this->read(c))
            return false;
        while (c != '\0')
        {
            value.append(c);
            if (!this->read(c))
                return false;
        }
        return true;
    }

    bool readLine(String& value)
    {
        Stream& base = *this;
        char c = '\0';
        if (!this->read(c))
            return false;
        while (c != '\0' && c != '\n')
        {
            value.append(c);
            if (!this->read(c))
                return false;
        }
        return true;
    }

    bool write(const char& value)
    {
        Stream& base = *this;
        size_t wlen = base.write((void*)&value, 1);
        return wlen == 1;
    }

    bool write(const String& value)
    {
        Stream& base = *this;
        void* data = (void*)value.cstr();
        size_t size = value.length();
        size_t wlen = base.write(data, size);
        return wlen == size;
    }

    bool writeLine(const String& value)
    {
        Stream& base = *this;
        void* data = (void*)value.cstr();
        size_t size = value.length();
        size_t wlen = base.write(data, size);
        if (wlen != size)
            return false;
        return this->write('\n');
    }
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_STREAM_H_
