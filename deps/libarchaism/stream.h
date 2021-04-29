
#ifndef _EOKAS_DIALECT_STREAM_H_
#define _EOKAS_DIALECT_STREAM_H_

#include "header.h"
#include "string.h"

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
  i8_t readI8(const i8_t& defaultValue = 0);
  u8_t readU8(const u8_t& defaultValue = 0);
  i16_t readI16(const i16_t& defaultValue = 0);
  u16_t readU16(const u16_t& defaultValue = 0);
  i32_t readI32(const i32_t& defaultValue = 0);
  u32_t readU32(const u32_t& defaultValue = 0);
  i64_t readI64(const i64_t& defaultValue = 0);
  u64_t readU64(const u64_t& defaultValue = 0);
  f32_t readF32(const f32_t& defaultValue = 0);
  f64_t readF64(const f64_t& defaultValue = 0);
  bool readBool(const bool& defaultValue = 0);
  String readString(const String& defaultValue = String(""));

  bool read(i8_t& value);
  bool read(u8_t& value);
  bool read(i16_t& value);
  bool read(u16_t& value);
  bool read(i32_t& value);
  bool read(u32_t& value);
  bool read(i64_t& value);
  bool read(u64_t& value);
  bool read(f32_t& value);
  bool read(f64_t& value);
  bool read(bool& value);
  bool read(String& value);

  bool write(i8_t value);
  bool write(u8_t value);
  bool write(i16_t value);
  bool write(u16_t value);
  bool write(i32_t value);
  bool write(u32_t value);
  bool write(i64_t value);
  bool write(u64_t value);
  bool write(f32_t value);
  bool write(f64_t value);
  bool write(bool value);
  bool write(const String& value);
};

_EndNamespace(eokas)

#endif//_EOKAS_DIALECT_STREAM_H_
