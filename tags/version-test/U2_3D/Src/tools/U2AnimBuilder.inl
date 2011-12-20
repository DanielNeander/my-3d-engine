//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Key::Key()
:value(0.f, 0.f, 0.f, 0.f)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Key::Key(const D3DXVECTOR4& val) :
value(val)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Key::SetX(float x)
{
	this->value.x = x;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Key::SetY(float y)
{
	this->value.y = y;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Key::SetZ(float z)
{
	this->value.z = z;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Key::SetW(float w)
{
	this->value.w = w;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Key::Set(const D3DXVECTOR4& v)
{
	this->value = v;
}

//------------------------------------------------------------------------------
/**
*/
inline
const D3DXVECTOR4&
U2AnimBuilder::Key::Get() const
{
	return this->value;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Curve::Curve() :
keyArray(128, 2048),
ipolType(NONE),
isCollapsed(false),
firstKeyIndex(-1),
isAnimated(1),
collapsedKey(0.f, 0.f, 0.f, 0.f)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Curve::Curve(int numKeys, const Key& initKey) :
ipolType(NONE),
isCollapsed(true),
firstKeyIndex(-1),
isAnimated(1),
collapsedKey(0.f, 0.f, 0.f, 0.f)
{
	this->keyArray.Resize(numKeys);
	for(int i=0; i < keyArray.Size(); ++i)
	{
		keyArray.SetElem(i, (Key*)&initKey);
	}
}

//-------------------------------------------------------------------------------------------------
inline
U2AnimBuilder::Curve::~Curve()
{
	for(int i=0; i < keyArray.Size(); ++i)
	{
		U2_DELETE keyArray[i];
	}

	keyArray.RemoveAll();
}


//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetKey(int index, const Key& key)
{
	this->keyArray.SetSafeElem(index, (Key*)&key);
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Curve::GetNumKeys() const
{
	return this->keyArray.FilledSize();
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Key&
U2AnimBuilder::Curve::GetKeyAt(int index)
{
	return *keyArray.GetElem(index);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetIpolType(IpolType t)
{
	this->ipolType = t;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Curve::IpolType
U2AnimBuilder::Curve::GetIpolType() const
{
	return this->ipolType;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetFirstKeyIndex(int i)
{
	this->firstKeyIndex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Curve::GetFirstKeyIndex() const
{
	return this->firstKeyIndex;
}

//------------------------------------------------------------------------------
/**
This checks whether all keys in the curve are identical.
If yes, the Collapsed flag will be set, and the collapsed
key is set to the first key in the curve.

- 18-Oct-2004   floh    optimized curves have ipolType no longer set to NONE!
*/
inline
bool
U2AnimBuilder::Curve::Optimize()
{
	U2ASSERT(this->keyArray.Size() > 0);

	this->isCollapsed = false;
	int numKeys = this->keyArray.FilledSize();
	D3DXVECTOR4 minVec = this->keyArray[0]->Get();
	D3DXVECTOR4 maxVec = this->keyArray[0]->Get();
	int curKeyIndex;
	for (curKeyIndex = 1; curKeyIndex < numKeys; curKeyIndex++)
	{
		const D3DXVECTOR4& curVec = this->keyArray[curKeyIndex]->Get();
		minVec = U2Math::Minimum(minVec, curVec);
		maxVec = U2Math::Maximum(maxVec, curVec);
	}
	D3DXVECTOR4 dist = maxVec - minVec;
	U2DynString str;		
	str.Format(_T("min vec : %f, %f, %f, %f max vec : %f, %f, %f, %f\n"), minVec.x, minVec.y, minVec.z, minVec.w, 
		maxVec.x, maxVec.y, maxVec.z, maxVec.w);
	FILE_LOG(logDEBUG) << str.Str();
	float len = D3DXVec4Length(&dist);
	if (len < 0.001f)
	{
		this->isCollapsed = true;
		this->collapsedKey = this->keyArray[0]->Get();		
		U2DynString str;
		str.Format(_T("collasped key : %f, %f, %f, %f\n"), collapsedKey.x, 
			collapsedKey.y, collapsedKey.z, collapsedKey.w);
		FILE_LOG(logDEBUG) << str.Str();
		
	}
	return this->isCollapsed;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetCollapsed(bool b)
{
	this->isCollapsed = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
U2AnimBuilder::Curve::IsCollapsed() const
{
	return this->isCollapsed;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetCollapsedKey(const D3DXVECTOR4& key)
{
	this->collapsedKey = key;
}

//------------------------------------------------------------------------------
/**
*/
inline
const D3DXVECTOR4&
U2AnimBuilder::Curve::GetCollapsedKey() const
{
	return this->collapsedKey;
}

//------------------------------------------------------------------------------
/**
*/
inline
const TCHAR*
U2AnimBuilder::Curve::IpolType2String(IpolType t)
{
	switch (t)
	{
	case NONE:      return _T("none");
	case STEP:      return _T("step");
	case LINEAR:    return _T("linear");
	case QUAT:      return _T("quat");
	}
	U2ASSERT(false);
	return 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Curve::IpolType
U2AnimBuilder::Curve::String2IpolType(const TCHAR* str)
{
	U2ASSERT(str);
	if (0 == _tcscmp(str, _T("quat")))           return QUAT;
	else if (0 == _tcscmp(str, _T("step")))      return STEP;
	else if (0 == _tcscmp(str, _T("linear")))    return LINEAR;
	else                                    return NONE;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Group::Group() :
loopType(REPEAT),
startKey(0),
numKeys(0),
keyStride(0),
keyTime(0.0f),
fadeInFrames(0)
{
	// empty
}

inline 
U2AnimBuilder::Group::~Group()
{
	for(uint32 i=0; i < curveArray.Size(); ++i)
	{
		U2_DELETE curveArray[i];
	}

	curveArray.RemoveAll();
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::AddCurve(const Curve& curve)
{
	this->curveArray.AddElem((Curve*)&curve);
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Group::GetNumCurves() const
{
	return this->curveArray.FilledSize();
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Curve&
U2AnimBuilder::Group::GetCurveAt(int index) const
{
	return *this->curveArray[index];
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetLoopType(LoopType t)
{
	this->loopType = t;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Group::LoopType
U2AnimBuilder::Group::GetLoopType() const
{
	return this->loopType;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetFadeInFrames(float frames)
{
	this->fadeInFrames = frames;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
U2AnimBuilder::Group::GetFadeInFrames() const
{
	return this->fadeInFrames;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetStartKey(int start)
{
	this->startKey = start;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Group::GetStartKey() const
{
	return this->startKey;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetNumKeys(int num)
{
	this->numKeys = num;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Group::GetNumKeys() const
{
	return this->numKeys;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetKeyStride(int stride)
{
	this->keyStride = stride;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Group::GetKeyStride() const
{
	return this->keyStride;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetKeyTime(float t)
{
	this->keyTime = t;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
U2AnimBuilder::Group::GetKeyTime() const
{
	return this->keyTime;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Group::SetAnimJoints(U2PrimitiveVec<U2DynString>* joints)
{
	this->animJoints = joints;
}

//------------------------------------------------------------------------------
/**
*/
inline
const U2PrimitiveVec<U2DynString>&
U2AnimBuilder::Group::GetAnimJoints() const
{
	return *animJoints;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
U2AnimBuilder::Group::Validate() const
{
	// make sure all curves have the same number of keys as set in the anim group
	int numCurves = this->curveArray.FilledSize();
	int curveIndex;
	for (curveIndex = 0; curveIndex < numCurves; curveIndex++)
	{
		if (this->GetCurveAt(curveIndex).GetNumKeys() != this->GetNumKeys())
		{
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
/**
*/
inline
const TCHAR*
U2AnimBuilder::Group::LoopType2String(LoopType t)
{
	switch (t)
	{
	case CLAMP:     return _T("clamp");
	case REPEAT:    return _T("repeat");
	}
	U2ASSERT(false);
	return 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Group::LoopType
U2AnimBuilder::Group::String2LoopType(const TCHAR* str)
{
	U2ASSERT(str);
	if (0 == _tcscmp(_T("clamp"), str)) return CLAMP;
	else                           return REPEAT;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::AddGroup(Group& group)
{
	this->groupArray.AddElem(&group);
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::GetNumGroups() const
{
	return this->groupArray.FilledSize();
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimBuilder::Group&
U2AnimBuilder::GetGroupAt(int index)
{
	return *this->groupArray[index];
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2AnimBuilder::Curve::SetIsAnimated(int isAnim)
{
	this->isAnimated = isAnim;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2AnimBuilder::Curve::IsAnimated() const
{
	return this->isAnimated;
}

//------------------------------------------------------------------------------