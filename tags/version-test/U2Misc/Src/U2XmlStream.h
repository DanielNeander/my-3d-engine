/**************************************************************************************************
module	:	U2XmlStream
Author	:	Yun sangyong
Desc	:   Close 함수 Save 미처리.
*************************************************************************************************/

#pragma once 
#ifndef U2_XMLSTREAM_H
#define U2_XMLSTREAM_H

//------------------------------------------------------------------------------
/**
@class U2XmlStream
@ingroup Util

A stream provides read/write access to an XML file.

NOTE: since the stream is based on Nebula2's file routines, the stream
can be read from an npk archive, but cannot be written to npk archives!

(C) 2005 Radon Labs GmbH
*/
#include <U2Lib/Src/Memory/U2MemObj.h>


class U2MISC U2XmlStream : public U2MemObj
{
public:
	/// read/write mode flags
	enum Mode
	{
		InvalidMode,
		Read,
		Write,
		ReadWrite,
		String,
	};

	/// default constructor
	U2XmlStream();
	/// constructor with filename
	U2XmlStream(const U2DynString& fname);
	/// destructor
	virtual ~U2XmlStream();
	///returns the whole document as string
	void GetDocumentAsString(U2DynString& rsString);
	/// turn automatic UTF8 de/encoding on/off (default is off)
	void SetAutomaticUTF8Coding(bool b);
	/// get automatic UTF8 coding flag
	bool GetAutomaticUTF8Coding() const;
	/// set the filename of the stream
	void SetFilename(const U2DynString& n);
	/// get the filename of the stream
	const U2DynString& GetFilename() const;
	/// open the stream for reading or writing
	virtual bool Open(Mode mode);
	/// open string for parsing
	virtual bool OpenString(const U2DynString& n);
	/// close the stream
	virtual bool Close();
	/// return true if stream is open
	bool IsOpen() const;
	/// return true if new file has been created in Open
	bool FileCreated() const;

	/// begin a new node (write mode only)
	bool BeginNode(const U2DynString& nodeName);
	/// end current node (write mode only)
	void EndNode();

	/// return true if node identified by path exists
	bool HasNode(const U2DynString& p) const;
	/// get the short name of the current node
	U2DynString GetCurrentNodeName() const;
	/// get current node name as full path
	U2DynString GetCurrentNodePath() const;
	/// returns the line number of the current node
	int GetCurrentNodeLineNumber() const;
	/// set current node as path (read/write)
	void SetToNode(const U2DynString& p);
	/// set current node to first child node, return false if no child exists
	bool SetToFirstChild(const U2DynString& name = _T(""));
	/// set current node to next sibling node, returns false if no more children exists
	bool SetToNextChild(const U2DynString& name = _T(""));
	/// set current node to parent node, returns false if no parent exists
	bool SetToParent();
	/// set current node to an node, that has the attribute and the value
	void SetToNodeByAttribute(const U2DynString& nodeName, const U2DynString& attribute, const U2DynString& value);

	/// return true if current node has embedded text
	bool HasText() const;
	/// return the text embedded by the current node
	U2DynString GetText() const;

	/// return true if attribute exists on current node
	bool HasAttr(const U2DynString& name) const;
	/// return names of all attributes on current node
	U2ObjVec<U2DynString>& GetAttrs() const;

	/// set string attribute in current node
	void SetString(const U2DynString& name, const U2DynString& s);
	/// set bool attribute on current node
	void SetBool(const U2DynString& name, bool b);
	/// set int attribute on current node
	void SetInt(const U2DynString& name, int i);
	/// set float attribute on current node
	void SetFloat(const U2DynString& name, float f);
	/// set vector3 attribute on current node
	void SetVector3(const U2DynString& name, const D3DXVECTOR3& v);
	/// set D3DXVECTOR4 attribute on current node
	void SetVector4(const U2DynString& name, const D3DXVECTOR4& v);
	/// set matrix44 attribute on current node
	void SetMatrix44(const U2DynString& name, const D3DXMATRIX& m);

	/// get string attribute from current node
	U2DynString GetString(const U2DynString& name) const;
	/// get bool attribute from current node
	bool GetBool(const U2DynString& name) const;
	/// get int attribute from current node
	int GetInt(const U2DynString& name) const;
	/// get float attribute from current node
	float GetFloat(const U2DynString& name) const;
	/// get D3DXVECTOR3 attribute from current node
	D3DXVECTOR3 GetVector3(const U2DynString& name) const;
	/// get vector4 attribute from current node
	D3DXVECTOR4 GetVector4(const U2DynString& name) const;
	/// get matrix44 attribute from stream
	D3DXMATRIX GetMatrix44(const U2DynString& name) const;

	/// get optional string attribute from current node
	U2DynString GetOptionalString(const U2DynString& name, const U2DynString& defaulValue) const;
	/// get optional bool attribute from current node
	bool GetOptionalBool(const U2DynString& name, bool defaultValue) const;
	/// get optional int attribute from current node
	int GetOptionalInt(const U2DynString& name, int defaultValue) const;
	/// get optional float attribute from current node
	float GetOptionalFloat(const U2DynString& name, float defaultValue) const;
	/// get optional vector3 attribute from current node
	D3DXVECTOR3 GetOptionalVector3(const U2DynString& name, const D3DXVECTOR3& defaultValue) const;
	/// get optional vector4 attribute from current node
	D3DXVECTOR4 GetOptionalVector4(const U2DynString& name, const D3DXVECTOR4& defaultValue) const;
	/// get optional matrix44 attribute from stream
	D3DXMATRIX GetOptionalMatrix44(const U2DynString& name, const D3DXMATRIX& defaultValue) const;

private:
	/// find a node by path, handles relative paths as well
	TiXmlNode* FindNode(const U2DynString& path) const;

	///  helper function , to generate a string
	void AddNode2String(U2DynString& rsString, const TiXmlNode* pNode, int iIndention = 0);

	bool utf8Coding;
	U2DynString filename;
	Mode mode;
	bool fileCreated;
	TiXmlDocument* xmlDocument;
	TiXmlElement* curNode;
};

//------------------------------------------------------------------------------

#endif