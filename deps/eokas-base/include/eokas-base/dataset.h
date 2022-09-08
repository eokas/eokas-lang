
#ifndef  _EOKAS_BASE_DATASET_H_
#define  _EOKAS_BASE_DATASET_H_

#include "header.h"
#include "string.h"

_BeginNamespace(eokas)

#ifndef _RadixDataSetVersion
#define _RadixDataSetVersion 0x00010001
#endif//_RadixDataSetVersion

enum DataType
{
	eDataType_Raw,
	eDataType_Number,
	eDataType_Bool,
	eDataType_String
};
/*
============================================================================================
==== DataCell
============================================================================================
*/
class DataCell
{
public:
	DataCell();
	~DataCell();

public:
	u8_t* data() const;
	u16_t length() const;
	void setData(u8_t* data, u16_t length);
	void clear();

public:
	DataCell& operator=(const DataCell& cell);
	DataCell& operator=(const String& value);
	DataCell& operator=(char value);
	DataCell& operator=(u8_t value);
	DataCell& operator=(i16_t value);
	DataCell& operator=(u16_t value);
	DataCell& operator=(i32_t value);
	DataCell& operator=(u32_t value);
	DataCell& operator=(i64_t value);
	DataCell& operator=(u64_t value);
	DataCell& operator=(f32_t value);
	DataCell& operator=(f64_t value);
	DataCell& operator=(bool value);

	operator String();
	operator char();
	operator u8_t();
	operator i16_t();
	operator u16_t();
	operator i32_t();
	operator u32_t();
	operator i64_t();
	operator u64_t();
	operator f32_t();
	operator f64_t();
	operator bool();

private:
	u16_t mLength;
	u8_t* mData;
};
/*
============================================================================================
==== DataCol
============================================================================================
*/
class DataCol
{
	friend class DataSet;

public:
	DataCol(const String& name);
	~DataCol();

public:
	const String& name() const;
	const String& comm() const;
	u16_t length() const;
	const DataType& type() const;

	void setComm(const String& comm);
	void setLength(u16_t length);
	void setType(const DataType& type);

	DataCell* createCell();
	DataCell* selectCell(size_t rowId);
	void deleteCell(size_t rowId);
	void clearCells();

private:
	String mName;
	String mComm;
	DataType mType;
	u16_t mLength;
	std::deque<DataCell*> mCells;
};
/*
============================================================================================
==== DataRow
============================================================================================
*/
class DataRow
{
public:
	DataRow();
	~DataRow();

public:
	DataCell& operator[](const String& colName);

public:
	void setCell(const String& colName, DataCell* cell);
	DataCell* getCell(const String& colName);

private:
	std::map<String, DataCell*> mCells;
};
/*
============================================================================================
==== DataTable
============================================================================================
*/
class DataTable
{
	friend class DataSet;

public:
	DataTable(const String& name);
	~DataTable();

public:
	DataCol& operator[](const String& colName);
	DataRow operator[](size_t rowId);

public:
	const String& name() const;
	const String& comm() const;
	void setComm(const String& comm);

	size_t colCount() const;
	size_t rowCount() const;

	bool containsCol(const String& colName);
	DataCol* createCol(const String& colName);
	DataCol* selectCol(const String& colName);
	void deleteCol(const String& colName);

	DataRow createRow();
	DataRow selectRow(size_t rowId);
	void	deleteRow(size_t rowId);

	void clear();

private:
	String mName;
	String mComm;
	std::map<String, DataCol*> mCols;
	size_t mRowCount;
};
/*
============================================================================================
==== DataSet
============================================================================================
*/
class DataSet
{
public:
	DataSet();
	~DataSet();

public:
	DataTable& operator[](const String& tableName);

public:
	size_t tableCount() const;
	bool containsTable(const String& tableName);
	DataTable* createTable(const String& tableName);
	DataTable* selectTable(const String& tableName);
	void deleteTable(const String& tableName);

	void load(u8_t* bytes);
	void save(u8_t* bytes, size_t* length);
	void clear();

private:
	u32_t mVersion;
	std::map<String, DataTable*> mTables;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_DATASET_H_
