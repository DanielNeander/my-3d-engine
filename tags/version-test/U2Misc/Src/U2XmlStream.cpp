//#include <U2_3D/Src/U23DLibPCH.h>
#include <stdafx.h>
#include "U2XmlStream.h"

//------------------------------------------------------------------------------
/**
*/
U2XmlStream::U2XmlStream() :
mode(InvalidMode),
fileCreated(false),
xmlDocument(0),
curNode(0),
utf8Coding(false)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
U2XmlStream::U2XmlStream(const U2DynString& fname) :
filename(fname),
mode(InvalidMode),
xmlDocument(0),
curNode(0),
utf8Coding(false)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
U2XmlStream::~U2XmlStream()
{
	if (this->IsOpen())
	{
		this->Close();
	}
}

//------------------------------------------------------------------------------
/**
*/
void
U2XmlStream::SetAutomaticUTF8Coding(bool b)
{
	this->utf8Coding = b;
}

//------------------------------------------------------------------------------
/**
*/
bool
U2XmlStream::GetAutomaticUTF8Coding() const
{
	return this->utf8Coding;
}

//------------------------------------------------------------------------------
/**
*/
void
U2XmlStream::SetFilename(const U2DynString& n)
{
	this->filename = n;
}

//------------------------------------------------------------------------------
/**
*/
const U2DynString&
U2XmlStream::GetFilename() const
{
	return this->filename;
}

//------------------------------------------------------------------------------
/**
*/
bool
U2XmlStream::IsOpen() const
{
	return (0 != this->xmlDocument);
}

//------------------------------------------------------------------------------
/**
*/
bool
U2XmlStream::FileCreated() const
{
	return this->fileCreated;
}



//------------------------------------------------------------------------------
/**
Generates a string from the document
*/
void
U2XmlStream::GetDocumentAsString(U2DynString& rsString)
{
	rsString = "";
	AddNode2String(rsString, this->xmlDocument->FirstChild());
}


//------------------------------------------------------------------------------
/**
helper function , to generate a string
*/

void
U2XmlStream::AddNode2String(U2DynString& rsString, const TiXmlNode* pNode, int iIndention)
{
	const char* pcValue = pNode->Value();

	/*
	for (int i = 0; i < iIndention; ++i)
	{
	rsString += "\t";
	} */

	switch (pNode->Type())
	{
	case TiXmlNode::COMMENT:
		//assert(pNode->NoChildren()  &&  "Node Type COMMENT should not have children");
		rsString += "<!-- ";
		rsString += pcValue;
		rsString += " -->";
		break;

	case TiXmlNode::DECLARATION:
		//assert(pNode->NoChildren()  &&  "Node Type DECLARATION should not have children");
		rsString += "<?";
		rsString += pcValue;
		rsString += "?>";
		break;

	case TiXmlNode::DOCUMENT:
		// assert (false && "No Documents should be passed into this function; pass the root element instead");
		return;

	case TiXmlNode::ELEMENT:
		rsString += "<";
		rsString += pcValue;

		for (const TiXmlAttribute* pAttr = pNode->ToElement()->FirstAttribute(); pAttr!= 0; pAttr = pAttr->Next())
		{
			rsString += " ";
			rsString += pAttr->Name();
			rsString += "=\"";
			rsString += pAttr->Value();
			rsString += "\"";
		}

		if (pNode->FirstChild() && pNode->FirstChild()->NextSibling() == 0 &&
			pNode->FirstChild()->Type() == TiXmlNode::TEXT)
		{
			rsString += ">";
			rsString += pNode->FirstChild()->ToText()->Value();
			rsString += "</";
			rsString += pcValue;
			rsString += ">";

			return;
		}

		if (pNode->NoChildren())
		{
			rsString += "/>";
		}
		else
		{
			rsString += ">";
		}
		break;

	case TiXmlNode::TEXT:
		//assert(pNode->NoChildren()  &&  "Node Type TEXT should not have children");
		rsString += pcValue;
		break;

	case TiXmlNode::UNKNOWN:
		//assert(pNode->NoChildren()  &&  "Node Type UNKNOWN should not have children");
		rsString += "<";
		rsString += pcValue;
		rsString += ">";
		break;

	default:
		//assert (false && "Unknown Node type");
		return;
	}

	if (pNode->Type() == TiXmlNode::ELEMENT && !pNode->NoChildren())
	{
		for (const TiXmlNode* pChild = pNode->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
		{
			AddNode2String(rsString, pChild, iIndention + 1);
		}


		/*
		for (int i = 0; i < iIndention; ++i)
		{
		rsString += "\t";
		} */

		rsString += "</";
		rsString += pcValue;
		rsString += ">";
	}
}

//------------------------------------------------------------------------------
/**
Opens a string for parsing
*/
bool
U2XmlStream::OpenString(const U2DynString& n)
{
	U2ASSERT(!this->IsOpen());
	U2ASSERT(!this->filename.IsEmpty());
	U2ASSERT(0 == this->curNode);
	this->fileCreated = false;
	this->mode = String;

	// create xml document object
	this->xmlDocument = new TiXmlDocument;
	xmlDocument->SetValue(this->filename.Str());

	if (!n.IsEmpty())
	{
		// parse the string
		this->xmlDocument->Parse(n.Str());
		if (this->xmlDocument->Error())
		{
			//n_error("U2XmlStream::OpenString() Failed to parse string !");
			return false;
		}

		// set the current node to the root node
		this->curNode = this->xmlDocument->RootElement();
		return true;
	}
	// no empty string allowed
	return false;
}
//------------------------------------------------------------------------------
/**
Open the stream in read, write or read/write mode. In Read mode the
stream file must exist, otherwise the method will fail hard with an error. In
Write mode an existing file will be overwritten. In ReadWrite mode
an existing file will be read, may be modified and will be
written back on Close().
*/
bool
U2XmlStream::Open(Mode m)
{
	U2ASSERT(!this->IsOpen());
	U2ASSERT(!this->filename.IsEmpty());
	U2ASSERT(0 == this->curNode);
	this->fileCreated = false;
	this->mode = m;

	// check if the file already exists
	bool fileExists = U2FileMgr::Instance()->FileExists(this->filename);
	if (!fileExists && ((Write == this->mode) || (ReadWrite == this->mode)))
	{
		this->fileCreated = true;
	}

	// create xml document object
	this->xmlDocument = new TiXmlDocument;
	xmlDocument->SetValue(this->filename.Str());
	if ((Read == this->mode) || ((ReadWrite == this->mode) && fileExists))
	{
		// read existing xml document
		if (!this->xmlDocument->LoadFile())
		{
			FDebug("U2XmlStream::Open() Failed to read file '%s'!", this->filename.Str());
			return false;
		}

		// set the current node to the root node
		this->curNode = this->xmlDocument->RootElement();
	}
	else
	{
		// when in write mode, we add a declaration to the XML stream
		// (this creates the line <?xml version="1.0" encoding="UTF-8"?> to the file
		this->xmlDocument->InsertEndChild(TiXmlDeclaration("1.0", "UTF-8", ""));
	}

	return true;
}

//------------------------------------------------------------------------------
/**
Close the stream. When in Write or ReadWrite mode, the xml document
will be saved out before being destroyed.
*/
bool
U2XmlStream::Close()
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(!this->filename.IsEmpty());

	// if in write or read/write mode, save out the xml document
	if ((Write == this->mode) || (ReadWrite == this->mode))
	{
		//nFileServer2* fileServer = nFileServer2::Instance();

		//// make sure the path to the file exists
		//fileServer->MakePath(this->filename.ExtractDirName());

		//// FIXME: TinyXml doesn't support Nebula2 filenames for writing
		//U2DynString mangledPath = fileServer->ManglePath(this->filename);
		//if (!this->xmlDocument->SaveFile(mangledPath.Get()))
		//{
		//	n_delete(this->xmlDocument);
		//	this->xmlDocument = 0;
		//	FDebug("U2XmlStream::Close(): Failed to write file '%s'!", this->filename.Str());
		//	return false;
		//}
		return false;
	}

	// delete the xml document
	delete this->xmlDocument;
	this->xmlDocument = 0;
	this->curNode = 0;
	this->mode = InvalidMode;
	return true;
}

//------------------------------------------------------------------------------
/**
This method returns the line number of the current node.
*/
int
U2XmlStream::GetCurrentNodeLineNumber() const
{
	if (this->curNode)
	{
		return this->curNode->Row();
	}
	return 0;
}

//------------------------------------------------------------------------------
/**
This method finds an xml node by path name. It can handle absolute
paths and paths relative to the current node. All the usual file system
path conventions are valid: "/" is the path separator, "." is the
current directory, ".." the parent directory.
*/
TiXmlNode*
U2XmlStream::FindNode(const U2DynString& path) const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(path.Length() > 0);

	bool absPath = (path[0] == '/');
	U2ObjVec<U2DynString> tokens;
	path.Tokenize("/", tokens);

	TiXmlNode* node;
	if (absPath)
	{
		node = this->xmlDocument;
	}
	else
	{
		U2ASSERT(0 != this->curNode);
		node = this->curNode;
	}

	int i;
	int num = tokens.Size();
	for (i = 0; i < num; i++)
	{
		const U2DynString& cur = tokens[i];
		if ("." == cur)
		{
			// do nothing
		}
		else if (".." == cur)
		{
			// go to parent directory
			node = node->Parent();
			if (node == this->xmlDocument)
			{
				FDebug("U2XmlStream::FindNode(%s): path points above root node!", path.Str());
				return 0;
			}
		}
		else
		{
			// find child node
			node = node->FirstChild(cur.Str());
			if (0 == node)
			{
				return 0;
			}
		}
	}
	return node;
}

//------------------------------------------------------------------------------
/**
Begin a new node in write mode. The new node will be set as the current
node. Nodes may form a hierarchy. Make sure to finalize a node
with a corresponding call to EndNode()!
*/
bool
U2XmlStream::BeginNode(const U2DynString& name)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(Read != this->mode);
	if (0 == this->curNode)
	{
		// create the root node
		this->curNode = this->xmlDocument->InsertEndChild(TiXmlElement(name.Str()))->ToElement();
	}
	else
	{
		// create a child node
		this->curNode = this->curNode->InsertEndChild(TiXmlElement(name.Str()))->ToElement();
	}
	return true;
}

//------------------------------------------------------------------------------
/**
End a node in write mode. This will set the parent of the current node as
new current node so that correct hierarchical behavior is implemented.
*/
void
U2XmlStream::EndNode()
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(Read != this->mode);
	U2ASSERT(0 != this->curNode);

	TiXmlNode* parent = this->curNode->Parent();
	U2ASSERT(parent);
	if (parent == this->xmlDocument)
	{
		// we're back at the root
		this->curNode = 0;
	}
	else
	{
		this->curNode = parent->ToElement();
	}
}

//------------------------------------------------------------------------------
/**
This method returns true if the node identified by path exists. Path
follows the normal file system path conventions, "/" is the separator,
".." is the parent node, "." is the current node. An absolute path
starts with a "/", a relative path doesn't.
*/
bool
U2XmlStream::HasNode(const U2DynString& n) const
{
	return (this->FindNode(n) != 0);
}

//------------------------------------------------------------------------------
/**
Get the short name (without path) of the current node. Node that
(as a special case), that the name of the root node is the filename
of the XML document. This is a quirk of TinyXML and isn't handled
separately here.
*/
U2DynString
U2XmlStream::GetCurrentNodeName() const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	return U2DynString(this->curNode->Value());
}

//------------------------------------------------------------------------------
/**
This returns the full absolute path of the current node. Path components
are separated by slashes.
*/
U2DynString
U2XmlStream::GetCurrentNodePath() const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);

	// build bottom-up array of node names
	U2ObjVec<U2DynString> components;
	TiXmlNode* node = this->curNode;
	while (node != this->xmlDocument)
	{
		components.AddElem(node->Value());
		node = node->Parent();
	}

	// build top down path
	U2DynString path = "/";
	int i;
	for (i = components.FilledSize() - 1; i >= 0; --i)
	{
		path.Concatenate(components[i]);
		if (i > 0)
		{
			path.Concatenate("/");
		}
	}
	return path;
}

//------------------------------------------------------------------------------
/**
Set the node pointed to by the path string as current node. The path
may be absolute or relative, following the usual file system path
conventions. Separator is a slash.
*/
void
U2XmlStream::SetToNode(const U2DynString& p)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(!p.IsEmpty());
	this->curNode = this->FindNode(p)->ToElement();
	if (0 == this->curNode)
	{
		FDebug("U2XmlStream::SetToNode(%s): node to found!", p.Str());
	}
}

//------------------------------------------------------------------------------
/**
Sets the current node to the first child node. If no child node exists,
the current node will remain unchanged and the method will return false.
If name is a valid string, only child element matching the name will
be returned. If name is empty, all child nodes will be considered.
*/
bool
U2XmlStream::SetToFirstChild(const U2DynString& name)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	TiXmlElement* child = 0;
	if (name.IsEmpty())
	{
		child = this->curNode->FirstChildElement();
	}
	else
	{
		child = this->curNode->FirstChildElement(name.Str());
	}
	if (child)
	{
		this->curNode = child;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
Sets the current node to the next sibling. If no more children exist,
the current node will be reset to the parent node and the method will
return false. If name is a valid string, only child element matching the
name will be returned. If name is empty, all child nodes will be considered.
*/
bool
U2XmlStream::SetToNextChild(const U2DynString& name)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);

	TiXmlElement* sib = 0;
	if (name.IsEmpty())
	{
		sib = this->curNode->NextSiblingElement();
	}
	else
	{
		sib = this->curNode->NextSiblingElement(name.Str());
	}
	if (sib)
	{
		this->curNode = sib;
		return true;
	}
	this->SetToParent();
	return false;
}

//------------------------------------------------------------------------------
/**
Sets the current node to its parent. If no parent exists, the
current node will remain unchanged and the method will return false.
*/
bool
U2XmlStream::SetToParent()
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	TiXmlNode* parent = this->curNode->Parent();
	if (parent)
	{
		this->curNode = parent->ToElement();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
*/
void
U2XmlStream::SetToNodeByAttribute(const U2DynString& nodeName, const U2DynString& attribute, const U2DynString& value)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(!nodeName.IsEmpty());
	U2ASSERT(!attribute.IsEmpty());
	U2ASSERT(!value.IsEmpty());

	this->SetToNode(nodeName);
	while (this->GetString(attribute) != value)
	{
		if (this->SetToNextChild() == false)
		{
			return;
		}
	}
}

//------------------------------------------------------------------------------
/**
Return true if an attribute of the given name exists on the current
node.
*/
bool
U2XmlStream::HasAttr(const U2DynString& name) const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	U2ASSERT(!name.IsEmpty());
	return (0 != this->curNode->Attribute(name.Str()));
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as string. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
U2DynString
U2XmlStream::GetString(const U2DynString& name) const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	U2ASSERT(!name.IsEmpty());
	U2DynString str;
	const char* val = this->curNode->Attribute(name.Str());
	if (0 == val)
	{
		U2ASSERT(FALSE);
		FDebug("Stream: attribute '%s' doesn't exist on node '%s'!", name.Str(), this->curNode->Value());
	}
	else
	{
		str = val;
		if (this->utf8Coding)
		{
			str.UTF8toANSI();
		}
	}
	return str;
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as string. If the attribute doesn't
exist, the default value will be returned.
*/
U2DynString
U2XmlStream::GetOptionalString(const U2DynString& name, const U2DynString& defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetString(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as a bool. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
bool
U2XmlStream::GetBool(const U2DynString& name) const
{
	// NOTE: the check against 1 is for backward compatibility
	U2DynString value = this->GetString(name);
	return ("1" == value) || ("true" == value);
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as bool. If the attribute doesn't
exist, the default value will be returned.
*/
bool
U2XmlStream::GetOptionalBool(const U2DynString& name, bool defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetBool(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as int. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
int
U2XmlStream::GetInt(const U2DynString& name) const
{
	return this->GetString(name).ToInt();
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as int. If the attribute doesn't
exist, the default value will be returned.
*/
int
U2XmlStream::GetOptionalInt(const U2DynString& name, int defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetInt(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as float. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
float
U2XmlStream::GetFloat(const U2DynString& name) const
{
	return this->GetString(name).ToFloat();
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as float. If the attribute doesn't
exist, the default value will be returned.
*/
float
U2XmlStream::GetOptionalFloat(const U2DynString& name, float defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetFloat(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as vector3. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
D3DXVECTOR3
U2XmlStream::GetVector3(const U2DynString& name) const
{
	return this->GetString(name).ToVector3();
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as vector3. If the attribute doesn't
exist, the default value will be returned.
*/
D3DXVECTOR3
U2XmlStream::GetOptionalVector3(const U2DynString& name, 
								const D3DXVECTOR3& defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetVector3(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as vector4. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
D3DXVECTOR4
U2XmlStream::GetVector4(const U2DynString& name) const
{
	return this->GetString(name).ToVector4();
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as vector4. If the attribute doesn't
exist, the default value will be returned.
*/
D3DXVECTOR4 
U2XmlStream::GetOptionalVector4(const U2DynString& name, const D3DXVECTOR4& defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetVector4(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Return the provided attribute as matrix44. If the attribute does not exist
the method will fail hard (use HasAttr() to check for its existence).
*/
D3DXMATRIX
U2XmlStream::GetMatrix44(const U2DynString& name) const
{
	U2DynString matrixString = this->GetString(name);
	U2ObjVec<U2DynString> tokens;
	matrixString.Tokenize(";", tokens);
	U2ASSERT(tokens.FilledSize() == 4);
	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	m._11 = tokens[0].ToVector3().x;
	m._12 = tokens[0].ToVector3().y;
	m._13 = tokens[0].ToVector3().z;	
	
	m._21 = tokens[1].ToVector3().x;
	m._22 = tokens[1].ToVector3().y;
	m._22 = tokens[1].ToVector3().z;

	m._31 = tokens[2].ToVector3().x;
	m._32 = tokens[2].ToVector3().y;
	m._33 = tokens[2].ToVector3().z;

	m._41 = tokens[2].ToVector3().x;
	m._42 = tokens[2].ToVector3().y;
	m._43 = tokens[2].ToVector3().z;
	
	
	return m;
}

//------------------------------------------------------------------------------
/**
Return the provided optional attribute as matrix44. If the attribute doesn't
exist, the default value will be returned.
*/
D3DXMATRIX
U2XmlStream::GetOptionalMatrix44(const U2DynString& name, const D3DXMATRIX& defaultValue) const
{
	if (this->HasAttr(name))
	{
		return this->GetMatrix44(name);
	}
	return defaultValue;
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a string value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetString(const U2DynString& name, const U2DynString& value)
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	U2ASSERT(!name.IsEmpty());
	U2ASSERT(!value.IsEmpty());
	U2ASSERT((Write == this->mode) || (ReadWrite == this->mode || (String == this->mode)));
	if (this->utf8Coding)
	{
		U2DynString utf8 = value;
		utf8.UTF8toANSI();
		this->curNode->SetAttribute(name.Str(), utf8.Str());
	}
	else
	{
		this->curNode->SetAttribute(name.Str(), value.Str());
	}
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a bool value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetBool(const U2DynString& name, bool b)
{
	U2DynString s;
	s.SetBool(b);
	this->SetString(name, s);
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a int value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetInt(const U2DynString& name, int i)
{
	U2DynString s;
	s.SetInt(i);
	this->SetString(name, s);
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a float value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetFloat(const U2DynString& name, float f)
{
	U2DynString s;
	s.SetFloat(f);
	this->SetString(name, s);
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a vector3 value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetVector3(const U2DynString& name, const D3DXVECTOR3& v)
{
	U2DynString s;
	s.SetVector3(v);
	this->SetString(name, s);
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a vector4 value. The stream must be
in Write or ReadWrite mode for this.
*/
void
U2XmlStream::SetVector4(const U2DynString& name, const D3DXVECTOR4& v)
{
	U2DynString s;
	s.SetVector4(v);
	this->SetString(name, s);
}

//------------------------------------------------------------------------------
/**
Set the provided attribute to a matrix44 value.  The stream must be
in Write or ReadWrite mode for this. Note: the matrix will be written
as 4x3 (without rightmost column), so it's not suitable for projection
matrices.
*/
void
U2XmlStream::SetMatrix44(const U2DynString& name, const D3DXMATRIX& m)
{
	U2DynString x,y,z,p;
	x.SetVector3(*(D3DXVECTOR3*)&m.m[0]);
	y.SetVector3(*(D3DXVECTOR3*)&m.m[1]);
	z.SetVector3(*(D3DXVECTOR3*)&m.m[2]);
	p.SetVector3(*(D3DXVECTOR3*)&m.m[3]);
	U2DynString matrixString;
	matrixString.Format(_T("%s;%s;%s;%s"), x.Str(), y.Str(), z.Str(), p.Str());
	this->SetString(name, matrixString);
}

//------------------------------------------------------------------------------
/**
Returns the names of all attributes on the node.
*/
U2ObjVec<U2DynString>&
U2XmlStream::GetAttrs() const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	U2ObjVec<U2DynString>* attrs = U2_NEW U2ObjVec<U2DynString>(32, 32);
	TiXmlAttribute* attr = this->curNode->FirstAttribute();
	if (attr) do
	{
		attrs->AddElem(attr->Name());
	}
	while ((attr = attr->Next()));
	return *attrs;
}

//------------------------------------------------------------------------------
/**
Returns true if the current node has embedded text (in the form
\<Node\>Embedded Text\</Node\>)
*/
bool
U2XmlStream::HasText() const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	TiXmlNode* child = this->curNode->FirstChild();
	return child && (child->Type() == TiXmlNode::TEXT);
}

//------------------------------------------------------------------------------
/**
Return the embedded text of the current node as a string. See
HasText() for details.
*/
U2DynString
U2XmlStream::GetText() const
{
	U2ASSERT(this->IsOpen());
	U2ASSERT(this->curNode);
	TiXmlNode* child = this->curNode->FirstChild();
	U2ASSERT(child->Type() == TiXmlNode::TEXT);
	return U2DynString(child->Value());
}

