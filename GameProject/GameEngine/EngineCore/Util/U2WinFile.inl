//------------------------------------------------------------------------------
/**
*/
inline
bool
U2WinFile::IsOpen() const
{
	return this->isOpen;
}

//------------------------------------------------------------------------------
/**
@return     the current line number (only valid with GetS()/PutS())
*/
inline
int
U2WinFile::GetLineNumber() const
{
	return this->lineNumber;
}

//------------------------------------------------------------------------------
/**
@param   val    a 32 bit int value
@return         number of bytes written
*/
inline
int
U2WinFile::PutInt(int val)
{
	return this->Write(&val, sizeof(val));
}

//------------------------------------------------------------------------------
/**
@param  val     a 16 bit int value
@return         number of bytes written
*/
inline
int
U2WinFile::PutShort(short val)
{
	return this->Write(&val, sizeof(val));
}

//------------------------------------------------------------------------------
/**
@param  val     a 8 bit int value
@return         number of bytes written
*/
inline
int
U2WinFile::PutChar(char val)
{
	return this->Write(&val, sizeof(val));
}

//------------------------------------------------------------------------------
/**
@param  val     a 32 bit float value
@return         number of bytes written
*/
inline
int
U2WinFile::PutFloat(float val)
{
	return this->Write(&val, sizeof(val));
}

//------------------------------------------------------------------------------
/**
@param  val     a 64 bit double value
@return         number of bytes written
*/
inline
int
U2WinFile::PutDouble(double val)
{
	return this->Write(&val, sizeof(val));
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
int
U2WinFile::GetInt()
{
	int val = 0;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
short
U2WinFile::GetShort()
{
	short val;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
uint16
U2WinFile::GetUShort()
{
	uint16 val;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
char
U2WinFile::GetChar()
{
	char val;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
float
U2WinFile::GetFloat()
{
	float val;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------
/**
@return     the value
*/
inline
double
U2WinFile::GetDouble()
{
	double val;
	this->Read(&val, sizeof(val));
	return val;
}

//------------------------------------------------------------------------------