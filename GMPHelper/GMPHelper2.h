#pragma once
#pragma warning(disable : 4996)
#include "EuroScopePlugIn.h"
#include "pch.h"
#include <vector>
class CTOTData
{
public:
	CTOTData(){}
	EuroScopePlugIn::CFlightPlan flightplan;
	CTime CTOT, TOBT;
	int sequence;
	bool manual = false;
	bool CTOTData::operator==(const CTOTData& rhs)
	{
		if ((this->flightplan.GetCallsign() == rhs.flightplan.GetCallsign()) && this->CTOT == rhs.CTOT && this->sequence == rhs.sequence && this->TOBT == rhs.TOBT)
		{
			return true;
		}
		return false;
	}
	bool CTOTData::operator<(const CTOTData& rhs)
	{
		if (this->CTOT < rhs.CTOT)
		{
			return true;
		}
		return false;
	}

};
class CGMPHelper :
	public EuroScopePlugIn::CPlugIn
{
public:
	EuroScopePlugIn::CFlightPlanList  m_TOSequenceList;
	std::vector<CTOTData> m_sequence;
	int lastsequence;
	CGMPHelper(void);


	//---~CPrecisionApproachPlugIn-------------------------------------

	virtual ~CGMPHelper(void);
	inline  virtual bool    CGMPHelper::OnCompileCommand(const char * sCommandLine);
	virtual void CGMPHelper::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan);
	virtual void    CGMPHelper::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
		EuroScopePlugIn::CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int * pColorCode,
		COLORREF * pRGB,
		double * pFontSize);

	int     _SelectAcIndex(EuroScopePlugIn::CFlightPlan flightplan);
	virtual void    CGMPHelper::OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area);

	void assignCTOT(bool asap, EuroScopePlugIn::CFlightPlan);
	void CGMPHelper::updateList();
	CTimeSpan CGMPHelper::getIncrement(EuroScopePlugIn::CFlightPlan fp1, EuroScopePlugIn::CFlightPlan fp2);
	void CGMPHelper::recalculateCTOT(CTOTData inserted);
};
