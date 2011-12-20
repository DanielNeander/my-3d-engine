/************************************************************************
                                                                     
************************************************************************/
#ifndef U2_LIB_DEFINE_H
#define U2_LIB_DEFINE_H


#define SAFE_DELETE(p)	if((p)) delete (p); (p) = NULL;
#define SAFE_DELETE_ARRAY(p) if((p)) delete [] (p); (p) = NULL;


#endif