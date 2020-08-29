#pragma once
#pragma warning(disable : 4996)
#include "EuroScopePlugIn.h"
#include "pch.h"
#include <vector>
#include "csv.h"
class CTOTData
	//this class is used as a storage. Each aircraft that gets a ctot assigned will be put in a CTOTData object. It contains the flightplan the CTOT and TOBT a switch if it was manually assigned
{
public:
	CTOTData(){}
	EuroScopePlugIn::CFlightPlan flightplan;
	CTime CTOT, TOBT;
	int sequence;
	bool manual = false;
	bool CTOTData::operator==(const CTOTData& rhs)
		//overwriting the == and < operator to make use of the STL algorithms for sorting and finding lateron
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
	static bool CTOTData::test()
	{
		return true;
	}

};
class RouteTo
	//This class stores the different standard routes to a destination icao.
{
public:
	std::string mICAO, mDestname, mLevelR, mRoute, mRouteForced;
	std::vector<std::string> endpoints;
	RouteTo(std::string ICAO, std::string destname, std::string LevelR, std::string Route)
	{
		mICAO = ICAO;
		mDestname = destname;
		mLevelR = LevelR;
		mRoute = Route;
		endpoints.push_back("ALPOB");
		endpoints.push_back("GOLGU");
		endpoints.push_back("TUMAK");
		endpoints.push_back("GABKO");
		endpoints.push_back("ASMUK");
		endpoints.push_back("UKRAG");
		endpoints.push_back("RIKOP");
		endpoints.push_back("KHM");
		endpoints.push_back("ALKAN");
		endpoints.push_back("OBSAS");
		endpoints.push_back("TARDI");
		endpoints.push_back("LALDO");
		endpoints.push_back("AFNAN");
		CTOTData::test();
		this->calculateForcedRoute(Route);
	}

	void calculateForcedRoute(std::string Route)
	{
		for (auto point : endpoints)
		{
			auto found = Route.find(point);
			if (found != std::string::npos)
			{
				if(point == "OBSAS" &&this->mICAO == "OBBI")
				{
					mRouteForced = Route;
					break;
				}
				if(point == "AFNAN" && this->mICAO =="OTHH")
				{
					mRouteForced = Route;
					break;
				}
				mRouteForced = Route.substr(1, found + 4);
			}
		}
	}
	bool isCruiseValid(int Flightlevel)
	{
		if (this->mLevelR == "ODD")
		{
			if ((Flightlevel / 1000) % 2 == 1) return true;
			else return false;
		}
		if (this->mLevelR == "EVEN")
		{
			if ((Flightlevel / 1000) % 2 == 0) return true;
			else return false;
		}
		if (this->mLevelR == "MAXFL260EVEN")
		{
			if (((Flightlevel / 1000) % 2 == 0) && Flightlevel <= 26000) return true;
			else return false;
		}
		return false;
	}
	bool isRouteValid(std::string Route)
	{
		auto check = Route.find(mRouteForced);
		if (check == std::string::npos)
			return false;
		else return true;
	}

};
class RouteData
	//this Class holds all RouteTos
{
public:
	RouteData(){}
	std::vector<RouteTo> Routes;
	std::vector<std::string> icaos;

	std::vector<RouteTo> getDatafromICAO(std::string icao)
	{
		std::vector<RouteTo> routes;
		for (auto temp : Routes)
		{
			if (icao == temp.mICAO)
				routes.push_back(temp);
		}
		return routes;
	}
};

class CGMPHelper :
	//The class that holds all our functions 
	public EuroScopePlugIn::CPlugIn
{
public:
	//the list displayed in euroscope
	EuroScopePlugIn::CFlightPlanList  m_TOSequenceList;
	//this vector holds a CTOTData for each aircraft
	std::vector<CTOTData> m_sequence;
	CGMPHelper(void);



	virtual ~CGMPHelper(void);

	inline  virtual bool    CGMPHelper::OnCompileCommand(const char * sCommandLine);
	/*
	This function overrides a Euroscope function. If you type ".showtolist" in the euroscope textbox it will show the t/o sequence list
	Input: sCommandLine (the textbox string)
	*/
	bool fileExists(const std::string& filename)
	{
		struct stat buf;
		if (stat(filename.c_str(), &buf) != -1)
		{
			return true;
		}
		return false;
	}
	virtual void CGMPHelper::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan);
	/*
	This function overrides a Euroscope function. It makes sure that when a user disconnects, that his flightplan is deleted from our list and from the sequence
	Input: FlightPlan (the flightplan of the disconnected user)
	*/

	virtual void    CGMPHelper::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
		EuroScopePlugIn::CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int * pColorCode,
		COLORREF * pRGB,
		double * pFontSize);
	/*This function overrides a Euroscope function. It determines what value to show for our new TAG items "CTOT" and "TOBT" and is called automatically by euroscope every few seconds
	  detailed info on the input and output values can be found in the EuroScopePlugIn.h header
	*/

	int     _SelectAcIndex(EuroScopePlugIn::CFlightPlan flightplan);
	/* This function searches our sequence for the input "flightplan" and returns the index
	*/

	virtual void    CGMPHelper::OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area);
	/*This function overrides a Euroscope function. It handles the user interface interaction. So it shows the popup menu on left click on the respective TAG fields and so on 
	  detailed info on the input and output values can be found in the EuroScopePlugIn.h header
	*/

	void assignCTOT(bool asap, EuroScopePlugIn::CFlightPlan);
	/*This function is called when a CTOT should be assigned to an aircraft. It constructs the new CTOTData object and redoes the sequence
		Input: bool asap (if an aircraft should be assigned a ctot that is as close to now as possible or at the end of the sequence)
			   CFlightPlan flightplan (the corresponding flightplan)
	*/

	void CGMPHelper::updateList();
	//This function is called from various other functions to do housekeeping on the actual euroscope list

	CTimeSpan CGMPHelper::getIncrement(EuroScopePlugIn::CFlightPlan fp1, EuroScopePlugIn::CFlightPlan fp2);
	/*This function is the heart of the implementation. It determines which separation fp2 needs to maintain to the preceeding aircraft fp1.
	  It takes into account the sids and wake turbulence category of the aircraft
	*/

	void CGMPHelper::recalculateCTOT(CTOTData inserted);
	/* This function is called when we change the order of the sequence either by assigning an aircraft and asap ctot or by manually assigning one.
	   The function recalculates all CTOTs that follow the "inserted" so the modified one.
	*/
};
