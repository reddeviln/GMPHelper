#pragma once
#include "pch.h"
#include "GMPHelper2.h"
#include <string>
#include <ctime>
#include <algorithm>




#define MY_PLUGIN_NAME      "GMPHelper VATSIM OMAE"
#define MY_PLUGIN_VERSION   "1.0"
#define MY_PLUGIN_DEVELOPER "Nils Dornbusch"
#define MY_PLUGIN_COPYRIGHT "Free to be distributed as source code"
#define MY_PLUGIN_VIEW      ""

const   int     TAG_ITEM_CTOT = 1121312;
const   int     TAG_ITEM_Sequence = 212312;
const   int     TAG_ITEM_TOBT = 312312;

const   int     TAG_FUNC_CTOT_MANUAL = 1;   // for the TAGs
const   int     TAG_FUNC_CTOT_ASSIGN = 2;   // for the TAGs

const   int     TAG_FUNC_CTOT_MANUAL_FINISH = 10;  // when editing the point name
const   int     TAG_FUNC_CTOT_ASSIGN_SEQ = 11;  // for the popup list elements
const   int     TAG_FUNC_CTOT_ASSIGN_ASAP = 13;
const   int     TAG_FUNC_CTOT_CLEAR = 12341;



//---CGMPHelper------------------------------------------

CGMPHelper::CGMPHelper(void)
	: CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		MY_PLUGIN_NAME,
		MY_PLUGIN_VERSION,
		MY_PLUGIN_DEVELOPER,
		MY_PLUGIN_COPYRIGHT)
{
	
	RegisterTagItemType("CTOT", TAG_ITEM_CTOT);
	RegisterTagItemType("T/O sequence", TAG_ITEM_Sequence);
	RegisterTagItemType("TOBT", TAG_ITEM_TOBT);

	RegisterTagItemFunction("Assign CTOT", TAG_FUNC_CTOT_ASSIGN);
	RegisterTagItemFunction("Edit CTOT", TAG_FUNC_CTOT_MANUAL);

	m_TOSequenceList = RegisterFpList("T/O Sequence List");
	if (m_TOSequenceList.GetColumnNumber() == 0)
	{
		// fill in the default columns
		m_TOSequenceList.AddColumnDefinition("C/S", 10, true, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
		m_TOSequenceList.AddColumnDefinition("CTOT", 4, true, NULL, TAG_ITEM_CTOT,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
		m_TOSequenceList.AddColumnDefinition("#", 2, true, NULL, TAG_ITEM_Sequence,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
		m_TOSequenceList.AddColumnDefinition("W", 1, true, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_AIRCRAFT_CATEGORY,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
			NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
		
	}// no columns from settings
}


//---~CGMPHelper-----------------------------------------

CGMPHelper::~CGMPHelper(void)
{
}

void    CGMPHelper::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
	EuroScopePlugIn::CRadarTarget RadarTarget,
	int ItemCode,
	int TagData,
	char sItemString[16],
	int * pColorCode,
	COLORREF * pRGB,
	double * pFontSize)
{
	int     idx;


	// only for flight plans
	if (!FlightPlan.IsValid())
		return;

	// get the AC index
	if ((idx = _SelectAcIndex(FlightPlan)) < 0)
		return;

	// switch by the code
	switch (ItemCode)
	{
	case TAG_ITEM_CTOT:
	{	
		CTime CTOT = m_sequence[idx].CTOT;
		CString temp1;
		tm t;
		CTOT.GetGmtTm(&t);
		temp1.Format("%.4d", t.tm_hour*100+t.tm_min);
		strcpy(sItemString, temp1);
		updateList();
		break; 
	}

	case TAG_ITEM_TOBT:
	{
		CTime TOBT = m_sequence[idx].TOBT;
		CString temp1;
		tm t;
		TOBT.GetGmtTm(&t);
		temp1.Format("%.4d", t.tm_hour * 100 + t.tm_min);
		strcpy(sItemString, temp1);
		break;
	}
	case TAG_ITEM_Sequence:
	{
		int seq = m_sequence[idx].sequence;
		CString temp3;
		temp3.Format("%d", seq);
		strcpy(sItemString, temp3);
		break;
	}
	}
}// switch by the code

	int     CGMPHelper::_SelectAcIndex(EuroScopePlugIn::CFlightPlan flightplan)
	{
		// search
		int i = 0;
		if (m_sequence.empty())
		{
			return -1;
		}
		for (CTOTData test : m_sequence)
		{
			try {
				CString temp1 = test.flightplan.GetCallsign();
				CString temp2 = flightplan.GetCallsign();
				if (temp1.Compare(temp2) == 0)
				{
					return  i;
				}
				i++;
			}
			catch (const std::exception &e) { continue; }
		}
		return -1;
	}
	void    CGMPHelper::OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area)
	{
		EuroScopePlugIn::CFlightPlan  fp;
		int                             i;
		CString                         str;


		// get the ASEL AC
		fp = FlightPlanSelectASEL();
		if (!fp.IsValid())
			return;

		// select it from  my list
		int idx = _SelectAcIndex(fp);
		

		// switch by the function ID
		switch (FunctionId)
		{
		case TAG_FUNC_CTOT_MANUAL: // TAG function
		{
			CString str;
			if (idx >= 0)
			{
				str.Format("%d", "");
			}
			// open a popup editor there
			OpenPopupEdit(Area,
				TAG_FUNC_CTOT_MANUAL_FINISH,
				str);
		break;
		}
		case TAG_FUNC_CTOT_MANUAL_FINISH: // when finished editing
		{
			CTime empty,temp;
			temp = CTime::GetCurrentTime();
			tm t;
			temp.GetGmtTm(&t);
			int input = atoi(sItemString);
			// simply save the value
			if (idx >= 0)
			{
				try {
					m_sequence[idx].CTOT = CTime(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday, input / 100, input - (input / 100) * 100, 0);
					m_sequence[idx].TOBT = m_sequence[idx].CTOT - CTimeSpan(0, 0, 15, 0);
					std::sort(m_sequence.begin(), m_sequence.end());
				}
				catch(const std::exception &e)
				{ }
				break;
			}
			else {
				CTOTData temp;
				temp.flightplan = fp;
				try {
				temp.CTOT = CTime(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday, input / 100, input - (input / 100) * 100, 0);
				temp.TOBT = temp.CTOT - CTimeSpan(0, 0, 15, 0);
				m_sequence.push_back(temp);
				std::sort(m_sequence.begin(), m_sequence.end());
				}
				catch (const std::exception &e)
				{
				}
			}
			
			break;
		}
		case TAG_FUNC_CTOT_ASSIGN: // TAG function

			// start a popup list
			OpenPopupList(Area, "Assign CTOT", 1);


			AddPopupListElement("Assign in sequence", "", TAG_FUNC_CTOT_ASSIGN_SEQ);
			AddPopupListElement("Assign ASAP", "", TAG_FUNC_CTOT_ASSIGN_ASAP);


			// finally add a fixed element to clear the ctot

			AddPopupListElement("Clear", "",
				TAG_FUNC_CTOT_CLEAR);
			break;

		case TAG_FUNC_CTOT_ASSIGN_SEQ: // a value is selected from the popup list

			CGMPHelper::assignCTOT(false, fp);

			break;

		case TAG_FUNC_CTOT_ASSIGN_ASAP: // a value is selected from the popup list

			CGMPHelper::assignCTOT(true, fp);

			break;

		case TAG_FUNC_CTOT_CLEAR: // clear the waiting time

			// simply clear
			if (idx < 0) break;
			if (std::size(m_sequence) == idx + 1)
			{
				m_latestCTOT = m_sequence[idx].CTOT;
				m_latestfp = m_sequence[idx].flightplan;
			}
			m_sequence.erase(std::remove(m_sequence.begin(), m_sequence.end(), m_sequence.at(_SelectAcIndex(fp))), m_sequence.end());
			m_TOSequenceList.RemoveFpFromTheList(fp);
			updateList();
			break;

		}// switch by the function ID
	}
	
	void CGMPHelper::assignCTOT(bool asap, EuroScopePlugIn::CFlightPlan flightplan)
	{
		CTime time = CTime::GetCurrentTime();
		CTime ctot;
		CTime tobt;
		CTOTData temp;
		if (m_sequence.empty()||asap)
		{
		
			temp.flightplan = flightplan;
			temp.sequence = -1;
			ctot = time + CTimeSpan(0, 0, 20, 0);
			tobt = ctot - CTimeSpan(0, 0, 15, 0);
			temp.CTOT = ctot;
			temp.TOBT = tobt;
			if (!asap) {
				m_latestCTOT = ctot;
				m_latestfp = flightplan;
			}
		}
		else 
		{
			if (m_latestfp.GetCallsign() == flightplan.GetCallsign())
			{
				return;
			}
			CTimeSpan increment = getIncrement(m_latestfp, flightplan);
			temp.flightplan = flightplan;
			temp.sequence = -1;
			ctot = max(time + CTimeSpan(0, 0, 20, 0), m_latestCTOT + increment);
			tobt = ctot - CTimeSpan(0, 0, 15, 0);
			temp.CTOT = ctot;
			temp.TOBT = tobt;
			m_latestCTOT = ctot;
			m_latestfp = flightplan;
			
		}
		
		m_sequence.push_back(temp);
		std::sort(m_sequence.begin(), m_sequence.end());
		if (asap)
		{
			recalculateCTOT(temp);
		}

	}
	void CGMPHelper::updateList()
	{
		for (CTOTData i: m_sequence)
		{
			m_TOSequenceList.AddFpToTheList(i.flightplan);
		}
		for (CTOTData i : m_sequence)
		{
			auto fp = i.flightplan;
			EuroScopePlugIn::CRadarTarget rt = fp.GetCorrelatedRadarTarget();
			if (rt.GetGS()>80)
			{
				m_TOSequenceList.RemoveFpFromTheList(fp);
				m_sequence.erase(std::remove(m_sequence.begin(), m_sequence.end(), m_sequence.at(_SelectAcIndex(fp))), m_sequence.end());
			}
		}
		std::sort(m_sequence.begin(), m_sequence.end());
		for (CTOTData &i : m_sequence)
		{
			i.sequence= _SelectAcIndex(i.flightplan)+1;
		}
	}

	void CGMPHelper::recalculateCTOT(CTOTData inserted)
		//recalculates the CTOT for each flightplan in sequence following and inculding "inserted"
	{
		//get the number in sequence for our inserted fp
		auto pos = std::find(m_sequence.begin(), m_sequence.end(), inserted);
		//safeguard if not found due to whatever 
		if (pos == m_sequence.end())
		{
			return;
		}
		//iterate over the sequence starting from "inserted"
		for (auto &it = pos; it != m_sequence.end(); ++it) 
		{
			CTOTData temp1 = *(it-1);
			CTOTData &temp2 = *it;
			CTimeSpan inc = getIncrement(temp1.flightplan, temp2.flightplan);
			temp2.CTOT = temp1.CTOT + inc;
			temp2.TOBT = temp2.CTOT - CTimeSpan(0, 0, 15, 0);
		}

	}
	CTimeSpan CGMPHelper::getIncrement(EuroScopePlugIn::CFlightPlan fp1, EuroScopePlugIn::CFlightPlan fp2)
		//returns the required difference in CTOT when fp2 is trailing fp1
	{
		CTimeSpan increment;
		CString sid1 = fp1.GetFlightPlanData().GetSidName();
		CString sid2 = fp2.GetFlightPlanData().GetSidName();
		char wtc1 = fp1.GetFlightPlanData().GetAircraftWtc();
		char wtc2 = fp2.GetFlightPlanData().GetAircraftWtc();
		switch (wtc1)
		{
		case 'J':
		{
			if (wtc2 == 'H')
			{
				increment = CTimeSpan(0, 0, 2, 0);
			}
			if (wtc2 == 'M' || wtc2 == 'L')
			{
				increment = CTimeSpan(0, 0, 3, 0);
			}
			break;
		}
		case 'H':
		{
			if (wtc2 == 'M' || wtc2 == 'L')
			{
				increment = CTimeSpan(0, 0, 2, 0);
			}
			else {

			}
			break;
		}
		case 'M':
		{
			if (wtc2 == 'L')
			{
				increment = CTimeSpan(0, 0, 2, 0);
			}
			break;
		}
		}
		if (increment == NULL)
		{
			if (sid1 != sid2)
			{
				increment = CTimeSpan(0, 0, 1, 0);
			}
			else
				increment = CTimeSpan(0, 0, 2, 0);
		}
		return increment;
	}






