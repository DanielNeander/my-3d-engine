// Copyright ?2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"
#include "SpeciesGrammar.h"

#pragma warning( push )
#pragma warning( disable : 6011 )

void Grammar::loadLevel(std::string filename)
{
	const char* pcsName;
	const char* pcsValue;
	std::string sTDF("tgtest.tdf");
	TiXmlAttribute *pAset;
	TiXmlDocument   XmlDoc( filename.c_str() );
	int levelID =0;
	int splitCount =0;
	int splitID =0;
	if ( XmlDoc.LoadFile() == false )
	{
		//load failed.  need assert
	}

	//
	// Find the "environment" element.
	//
	TiXmlElement* pTreeGrammarElement = XmlDoc.FirstChildElement();
	TiXmlElement* pGrammarElement=0;
	TiXmlElement* pPropertiesElement=0;
	TiXmlElement* pPropertyElement=0;
	TiXmlElement* pChildElement=0;
	TiXmlElement* pSiblingElement=0;

	assert( pTreeGrammarElement != NULL );

	if(strcmp( pTreeGrammarElement->Value(), "TreeGrammar" ) !=0)
	{
		//Not a valid Grammar file; need assertion

	}

	//Get Grammar Definition Values
	pGrammarElement = pTreeGrammarElement->FirstChildElement();
	if(strcmp(pGrammarElement->Value(),"GrammarElement") ==0)
	{
		pAset = pGrammarElement->FirstAttribute();
		pcsName  = pAset->Name();
		pcsValue = pAset->Value();
		if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"GDV"))
		{
			pPropertyElement=pGrammarElement->FirstChildElement();
			while (pPropertyElement != NULL && !strcmp(pPropertyElement->Value(), "Property") !=0 )
			{
				pAset = pPropertyElement->FirstAttribute();
				pcsName  = pAset->Name();
				pcsValue = pAset->Value();
				if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"Name"))
				{
					pAset = pAset->Next();
					pcsName  = pAset->Name();
					pcsValue = pAset->Value();
				}
				else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"LevelCount"))
				{
					pAset = pAset->Next();
					pcsName  = pAset->Name();
					pcsValue = pAset->Value();
					m_levelCount = pAset->IntValue();
					m_pLevels = new LevelDetail[m_levelCount];

				}

				pPropertyElement = pPropertyElement->NextSiblingElement();
			}

		}
	}

	//pSiblingElement = pElement->FirstChildElement();
	pGrammarElement = pGrammarElement->NextSiblingElement();
	while ( pGrammarElement != NULL && !strcmp( pGrammarElement->Value(), "GrammarElement" ) != 0 )
	{
		pPropertiesElement = pGrammarElement->FirstChildElement();
		while (pPropertiesElement != NULL && !strcmp(pPropertiesElement->Value(), "Properties") !=0 )
		{
			pAset = pPropertiesElement->FirstAttribute();
			pcsName  = pAset->Name();
			pcsValue = pAset->Value();
			if(!strcmp(pcsName,"GrammarToken")&&!strcmp(pcsValue,"Level"))
			{
				pPropertyElement=pPropertiesElement->FirstChildElement();
				while (pPropertyElement != NULL && !strcmp(pPropertyElement->Value(), "Property") !=0 )
				{
					pAset = pPropertyElement->FirstAttribute();
					pcsName  = pAset->Name();
					pcsValue = pAset->Value();
					if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"LevelID"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						levelID = pAset->IntValue();
						m_pLevels[levelID].LevelID = levelID;
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"tipPointCount"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].tipPointCount = pAset->IntValue();

					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"AxialBias"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsName,"MinAngle"))
						{
							m_pLevels[levelID].AxialBias.minAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						} 
						else if(!strcmp(pcsName,"MaxAngle"))
						{
							m_pLevels[levelID].AxialBias.maxAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						} 
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"Length"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].length = (float)(pAset->DoubleValue());
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"Diameter"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].diameter = (float)(pAset->DoubleValue());
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"taper"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].taper = (float)(pAset->DoubleValue());
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"dropAngle"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsName,"MinAngle"))
						{
							m_pLevels[levelID].dropAngle.minAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						} 
						else if(!strcmp(pcsName,"MaxAngle"))
						{
							m_pLevels[levelID].dropAngle.maxAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						} 
						else if(!strcmp(pcsName,"BiasAngle"))
						{
							m_pLevels[levelID].dropAngle.biasAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						}

					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"Heading"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsName,"value1"))
						{
							float x,y,z;
							x = (float)(pAset->DoubleValue());
							pAset = pAset->Next();
							y = (float)(pAset->DoubleValue());
							pAset = pAset->Next();
							z = (float)(pAset->DoubleValue());
							m_pLevels[levelID].heading.Heading = V3(x,y,z);						
						} else if(!strcmp(pcsName,"MinAngle"))
						{
							m_pLevels[levelID].heading.biasRange.minAngle = (float)(pAset->DoubleValue()/ RadianDegree);						
						} else if(!strcmp(pcsName,"MaxAngle"))
						{
							m_pLevels[levelID].heading.biasRange.maxAngle = (float)(pAset->DoubleValue()/ RadianDegree);
						} 

					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"branchType"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsValue,"TRUNK"))
						{
							m_pLevels[levelID].brnchType = branchType::TRUNK;
						}   
						else if(!strcmp(pcsValue,"BRANCH"))
						{
							m_pLevels[levelID].brnchType = branchType::BRANCH;
						}   
						else if(!strcmp(pcsValue,"STEM"))
						{
							m_pLevels[levelID].brnchType = branchType::STEM;
						}   
						else if(!strcmp(pcsValue,"LEAF"))
						{
							m_pLevels[levelID].brnchType = branchType::LEAF;
						}   
						else if(!strcmp(pcsValue,"FLOWER"))
						{
							m_pLevels[levelID].brnchType = branchType::FLOWER;
						}   
						else if(!strcmp(pcsValue,"FRUIT"))
						{
							m_pLevels[levelID].brnchType = branchType::FRUIT;
						}   
						else if(!strcmp(pcsValue,"BUD"))
						{
							m_pLevels[levelID].brnchType = branchType::BUD;
						}   
						else if(!strcmp(pcsValue,"NULL_BRANCH"))
						{
							m_pLevels[levelID].brnchType = branchType::NULL_BRANCH;
						} 
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"canopyType"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsValue,"PATCH"))
						{
							m_pLevels[levelID].cnpyType = canopyType::PATCH;
						}   
						else if(!strcmp(pcsValue,"HEX"))
						{
							m_pLevels[levelID].cnpyType = canopyType::HEX;
						}   
						else if(!strcmp(pcsValue,"PROCEDURAL"))
						{
							m_pLevels[levelID].cnpyType = canopyType::PROCEDURAL;
						}   
						else if(!strcmp(pcsValue,"NULL_CANOPY"))
						{
							m_pLevels[levelID].cnpyType = canopyType::NULL_CANOPY;
						}   
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"splitListcount"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						splitCount = pAset->IntValue();
						m_pLevels[levelID].splitList = new split[splitCount];

					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"PerturbFactor"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].perturbFactor = (float)(pAset->DoubleValue());
					}

					pPropertyElement = pPropertyElement->NextSiblingElement();
				}

			}else if(!strcmp(pcsName,"GrammarToken")&&!strcmp(pcsValue,"SplitList"))
			{
				pPropertyElement=pPropertiesElement->FirstChildElement();
				while (pPropertyElement != NULL && !strcmp(pPropertyElement->Value(), "Property") !=0 )
				{
					pAset = pPropertyElement->FirstAttribute();
					pcsName  = pAset->Name();
					pcsValue = pAset->Value();
					if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"splitID"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						splitID = pAset->IntValue();
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"probability"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].splitList[splitID].probability = (float)(pAset->DoubleValue());
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"splitCount"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						m_pLevels[levelID].splitList[splitID].splitCount = pAset->IntValue();
					}
					else if(!strcmp(pcsName,"Name")&&!strcmp(pcsValue,"type"))
					{
						pAset = pAset->Next();
						pcsName  = pAset->Name();
						pcsValue = pAset->Value();
						if(!strcmp(pcsValue,"ORDINARY"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::ORDINARY;
						}   
						else if(!strcmp(pcsValue,"REPEAT_ORDINARY"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::REPEAT_ORDINARY;
						}   
						else if(!strcmp(pcsValue,"SQUARE"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::SQUARE;
						}   
						else if(!strcmp(pcsValue,"OPPOSED"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::OPPOSED;
						}   
						else if(!strcmp(pcsValue,"UNIFORM"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::UNIFORM;
						}   
						else if(!strcmp(pcsValue,"SPEAR"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::SPEAR;
						}   
						else if(!strcmp(pcsValue,"REPEAT_SQUARE"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::REPEAT_SQUARE;
						}   
						else if(!strcmp(pcsValue,"REPEAT_OPPOSED"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::REPEAT_OPPOSED;
						}   
						else if(!strcmp(pcsValue,"CANOPY"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::CANOPY;
						}   
						else if(!strcmp(pcsValue,"ORDINARY_CANOPY"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::ORDINARY_CANOPY;
						}   
						else if(!strcmp(pcsValue,"REPEAT_CANOPY"))
						{
							m_pLevels[levelID].splitList[splitID].type = splitType::REPEAT_CANOPY;
						}   
					}
					pPropertyElement = pPropertyElement->NextSiblingElement();
				}
			}
			pPropertiesElement = pPropertiesElement->NextSiblingElement();
		}
		pGrammarElement = pGrammarElement->NextSiblingElement();
	}

}
/*  XML Grammar file copied here in case it gets lost
<TreeGrammar>
<GrammarElement Name="GDV">
<Property Name="Name" value="TG1"/>
<Property Name="LevelCount" value="6"/>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="0"/>
<Property Name="tipPointCount" value1="13"/>
<Property Name="AxialBias" MinAngle="30.0"/>
<Property Name="AxialBias" MaxAngle="330.0"/>
<Property Name="Length" value1="10.0"/>
<Property Name="Diameter" value1="3.3"/>
<Property Name="taper" value1="0.30"/>
<Property Name="dropAngle" MinAngle="10.0"/>
<Property Name="dropAngle" MaxAngle="25.0"/>
<Property Name="dropAngle" BiasAngle="50.0"/>
<Property Name="Heading" value1="0.0" value2="1.0" value3="0.0"/>
<Property Name="Heading" MinAngle="-5.0"/>
<Property Name="Heading" MaxAngle="5.0"/>
<Property Name="branchType" value1="TRUNK"/>
<Property Name="canopyType" value1="NULL_CANOPY"/>
<Property Name="splitListcount" value1="4"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1="0.25"/>
<Property Name="splitCount" value1="5"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="1"/>
<Property Name="probability" value1="0.05"/>
<Property Name="splitCount" value1="2"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="2"/>
<Property Name="probability" value1="0.15"/>
<Property Name="splitCount" value1="3"/>
<Property Name="type" value1="REPEAT_OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="3"/>
<Property Name="probability" value1="0.55"/>
<Property Name="splitCount" value1="2"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="1"/>
<Property Name="tipPointCount" value1="11"/>
<Property Name="AxialBias" MinAngle="30.0"/>
<Property Name="AxialBias" MaxAngle="330.0"/>
<Property Name="Length" value1="7.0"/>
<Property Name="Diameter" value1="2.0"/>
<Property Name="taper" value1="0.20"/>
<Property Name="dropAngle" MinAngle="35.0"/>
<Property Name="dropAngle" MaxAngle="50.0"/>
<Property Name="dropAngle" BiasAngle="40.0"/>
<Property Name="Heading" MinAngle="-20.0"/>
<Property Name="Heading" MaxAngle="20.0"/>
<Property Name="branchType" value1="BRANCH"/>
<Property Name="canopyType" value1="NULL_CANOPY"/>
<Property Name="splitListcount" value1="5"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1="0.05"/>
<Property Name="splitCount" value1="3"/>
<Property Name="type" value1="REPEAT_OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="1"/>
<Property Name="probability" value1="0.05"/>
<Property Name="splitCount" value1="3"/>
<Property Name="type" value1="REPEAT_OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="2"/>
<Property Name="probability" value1="0.25"/>
<Property Name="splitCount" value1="3"/>
<Property Name="type" value1="REPEAT_OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="3"/>
<Property Name="probability" value1="0.45"/>
<Property Name="splitCount" value1="2"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="4"/>
<Property Name="probability" value1="0.20"/>
<Property Name="splitCount" value1="2"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="2"/>
<Property Name="tipPointCount" value1="9"/>
<Property Name="AxialBias" MinAngle="30.0"/>
<Property Name="AxialBias" MaxAngle="330.0"/>
<Property Name="Length" value1="6.0"/>
<Property Name="Diameter" value1="1.0"/>
<Property Name="taper" value1="0.33"/>
<Property Name="dropAngle" MinAngle="35.0"/>
<Property Name="dropAngle" MaxAngle="50.0"/>
<Property Name="dropAngle" BiasAngle="40.0"/>
<Property Name="Heading" MinAngle="-20.0"/>
<Property Name="Heading" MaxAngle="20.0"/>
<Property Name="branchType" value1="BRANCH"/>
<Property Name="canopyType" value1="NULL_CANOPY"/>
<Property Name="splitListcount" value1="3"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1="0.25"/>
<Property Name="splitCount" value1="1"/>
<Property Name="type" value1="REPEAT_OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="1"/>
<Property Name="probability" value1="0.25"/>
<Property Name="splitCount" value1="1"/>
<Property Name="type" value1="OPPOSED"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="2"/>
<Property Name="probability" value1="0.5"/>
<Property Name="splitCount" value1="2"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="3"/>
<Property Name="tipPointCount" value1="7"/>
<Property Name="AxialBias" MinAngle="30.0"/>
<Property Name="AxialBias" MaxAngle="330.0"/>
<Property Name="Length" value1="4.0"/>
<Property Name="Diameter" value1="1.0"/>
<Property Name="taper" value1="0.50"/>
<Property Name="dropAngle" MinAngle="55.0"/>
<Property Name="dropAngle" MaxAngle="70.0"/>
<Property Name="dropAngle" BiasAngle="60.0"/>
<Property Name="Heading" MinAngle="-20.0"/>
<Property Name="Heading" MaxAngle="20.0"/>
<Property Name="branchType" value1="BRANCH"/>
<Property Name="canopyType" value1="NULL_CANOPY"/>
<Property Name="splitListcount" value1="2"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1="0.10"/>
<Property Name="splitCount" value1="1"/>
<Property Name="type" value1="REPEAT_CANOPY"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="1"/>
<Property Name="probability" value1="0.9f"/>
<Property Name="splitCount" value1="4"/>
<Property Name="type" value1="OPPOSED"/>
</Properties>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="4"/>
<Property Name="tipPointCount" value1="7"/>
<Property Name="AxialBias" MinAngle="30.0"/>
<Property Name="AxialBias" MaxAngle="330.0"/>
<Property Name="Length" value1="3.0"/>
<Property Name="Diameter" value1="0.5"/>
<Property Name="taper" value1="0.0"/>
<Property Name="dropAngle" MinAngle="70.0"/>
<Property Name="dropAngle" MaxAngle="90.0"/>
<Property Name="dropAngle" BiasAngle="75.0"/>
<Property Name="Heading" MinAngle="-40.0"/>
<Property Name="Heading" MaxAngle="40.0"/>
<Property Name="branchType" value1="STEM"/>
<Property Name="canopyType" value1="NULL_CANOPY"/>
<Property Name="splitListcount" value1="2"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1=""/>0.0
<Property Name="splitCount" value1="0"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="1"/>
<Property Name="probability" value1="1.0"/>
<Property Name="splitCount" value1="0"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
</GrammarElement>
<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1="5"/>
<Property Name="PerturbFactor" value1="2.0"/>
<Property Name="branchType" value1="NULL_BRANCH"/>
<Property Name="canopyType" value1="PATCH"/>
<Property Name="splitListcount" value1="1"/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1="0"/>
<Property Name="probability" value1="1.0"/>
<Property Name="splitCount" value1="0"/>
<Property Name="type" value1="ORDINARY"/>
</Properties>
</GrammarElement>
<!--<GrammarElement>
<Properties GrammarToken="Level">
<Property Name="LevelID" value1=""/>
<Property Name="tipPointCount" Value1=""/>
<Property Name="AxialBias" MinAngle=""/>
<Property Name="AxialBias" MaxAngle=""/>
<Property Name="Length" value1=""/>
<Property Name="Diameter" value1=""/>
<Property Name="taper" value1=""/>
<Property Name="dropAngle" MinAngle=""/>
<Property Name="dropAngle" MaxAngle=""/>
<Property Name="dropAngle" BiasAngle=""/>
<Property Name="Heading" Value1="" Value2="" Value3=""/>
<Property Name="Heading" MinAngle=""/>
<Property Name="Heading" MaxAngle=""/>
<Property Name="branchType" value1="TRUNK::BRANCH::STEM::NULL_BRANCH"/>
<Property Name="canopyType" value1="NULL_CANOPY::PATCH"/>
<Property Name="PerturbFactor" Value1=""/>      
<Property Name="splitListcount" value1=""/>
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1=""/>
<Property Name="probability" value1=""/>
<Property Name="splitCount" value1=""/>
<Property Name="type" value1="ORDINARY::OPPOSED::REPEAT_CANOPY::REPEAT_OPPOSED"/>     
</Properties>
<Properties GrammarToken="SplitList">
<Property Name="splitID" value1=""/>
<Property Name="probability" value1=""/>
<Property Name="splitCount" value1=""/>
<Property Name="type" value1="ORDINARY::OPPOSED::REPEAT_CANOPY::REPEAT_OPPOSED"/>     
</Properties>
</GrammarElement>-->

</TreeGrammar>
*/

#pragma warning( pop )
