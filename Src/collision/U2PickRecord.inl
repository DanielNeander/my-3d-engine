inline bool U2PickRecord::operator== (const U2PickRecord& record) const 
{
	return fT == record.fT;
}

//-------------------------------------------------------------------------------------------------
inline bool U2PickRecord::operator !=(const U2PickRecord& record) const 
{
	return fT != record.fT;
}

//-------------------------------------------------------------------------------------------------
inline bool U2PickRecord::operator<  (const U2PickRecord& record) const
{
	return fT < record.fT;
}

//-------------------------------------------------------------------------------------------------
inline bool U2PickRecord::operator<= (const U2PickRecord& record) const
{
	return fT <= record.fT;
}

//-------------------------------------------------------------------------------------------------
inline bool U2PickRecord::operator>  (const U2PickRecord& record) const
{
	return fT <= record.fT;
}

//-------------------------------------------------------------------------------------------------
inline bool U2PickRecord::operator>= (const U2PickRecord& record) const
{
	return fT <= record.fT;
}


