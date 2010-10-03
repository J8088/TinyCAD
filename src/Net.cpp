/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002,2003 Matt Pyne.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "stdafx.h"
#include <set>
#include <map>

#include "TinyCadView.h"
#include "TinyCad.h"
#include "special.h"
#include "library.h"
#include "net.h"
#include "TinyCadRegistry.h"
#include "diag.h"
#include "DlgPCBExport.h"
#include "MultiSheetDoc.h"
#include "TinyCadMultiDoc.h"

extern CDlgERCListBox theERCListBox;




////// Generate the net list for this design //////


void CTinyCadView::OnSpecialNet()
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.net"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy_s(ext, _T(".net"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space, _T(".net"));
		#endif
	}

	CDlgPCBExport dlg;
	dlg.m_Filename = szFile;

	if (dlg.DoModal() != IDOK)
		return;

	// Generate the net list file
	CNetList netlist;
	netlist.m_prefix_references = dlg.m_Prefix;
	netlist.WriteNetListFile( dlg.m_type, static_cast<CTinyCadMultiDoc*>(GetDocument()), dlg.m_Filename );

	// Now open the netlist for the user
	CTinyCadApp::EditTextFile( dlg.m_Filename );
}


////// Generate the SPICE output for this design //////


void CTinyCadView::OnSpecialCreatespicefile() 
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.net"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy(ext, _T(".net"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space, _T(".net"));
		#endif
	}

	CFileDialog dlg( FALSE, _T("*.net"), szFile, OFN_HIDEREADONLY,
		_T("SPICE (*.net)|*.net|All files (*.*)|*.*||"), AfxGetMainWnd() ); 

	if (dlg.DoModal() != IDOK)
	{
		return;
	}


	// Generate the SPICE file
	CNetList netlist;
	netlist.WriteSpiceFile( static_cast<CTinyCadMultiDoc*>(GetDocument()), dlg.GetPathName() );
	//TRACE("Spice netlist file path = %S\n",dlg.GetPathName());
	// Now open the Spice netlist for the user
	CTinyCadApp::EditTextFile( dlg.GetPathName() );
}



////// Check the design rules for this design //////


////// The ERCBox dialog //////


BOOL CDlgERCBox::OnInitDialog()
{

  // Copy the errortest into the dialog
  CheckDlgButton(ERC_DUPREF,theErrorTest.DupRef);
  CheckDlgButton(ERC_UNCONNECT,theErrorTest.UnConnect);
  CheckDlgButton(ERC_NOCONNECT,theErrorTest.NoConnect);

  CheckDlgButton(ERC_POWER,theErrorTest.Power);
  CheckDlgButton(ERC_OUTPUTTOPWR,theErrorTest.OutputPwr);
  CheckDlgButton(ERC_OUTPUT,theErrorTest.Output);
  CheckDlgButton(ERC_NOUTPUT,theErrorTest.NoOutput);
  CheckDlgButton(ERC_UNCONNECTED,theErrorTest.UnConnected);
  CheckDlgButton(ERC_MULTIPLENETNAMES,theErrorTest.MultipleNetNames);
  CheckDlgButton(ERC_NONCASEDISTINCT,theErrorTest.NonCaseDistinctNetNames);
  CheckDlgButton(ERC_UNASSIGNEDREFDES,theErrorTest.UnAssignedRefDes);

  return TRUE;
}


void CDlgERCBox::OnOK()
{

  // Copy the dialog into the errortest
  theErrorTest.DupRef		            =IsDlgButtonChecked(ERC_DUPREF)!=0;
  theErrorTest.UnConnect	            =IsDlgButtonChecked(ERC_UNCONNECT)!=0;
  theErrorTest.NoConnect	            =IsDlgButtonChecked(ERC_NOCONNECT)!=0;

  theErrorTest.Power		            =IsDlgButtonChecked(ERC_POWER)!=0;
  theErrorTest.OutputPwr	            =IsDlgButtonChecked(ERC_OUTPUTTOPWR)!=0;
  theErrorTest.Output		            =IsDlgButtonChecked(ERC_OUTPUT)!=0;
  theErrorTest.NoOutput		            =IsDlgButtonChecked(ERC_NOUTPUT)!=0;
  theErrorTest.UnConnected	            =IsDlgButtonChecked(ERC_UNCONNECTED)!=0;
  theErrorTest.MultipleNetNames         =IsDlgButtonChecked(ERC_MULTIPLENETNAMES) != 0;
  theErrorTest.NonCaseDistinctNetNames  =IsDlgButtonChecked(ERC_NONCASEDISTINCT) != 0;
  theErrorTest.UnAssignedRefDes         =IsDlgButtonChecked(ERC_UNASSIGNEDREFDES) != 0;

  EndDialog( IDOK );
}




////// the menu entry point for this special function //////


// This table determines the type of the netlist
//
const int ErcTable[7][7] = {
// Net   > Unknown		Passive,		Input,			Output,				TriState/BiDir		Power,			NoConnect,			  (Object)
		  {nUnknown,	nPassive,		nInput,			nOutput,			nBiDir,				nPower,			nNoConnect },		// Unknown
		  {nPassive,	nPassive,		nPassive,		nOutput,			nBiDir,				nPower,			nNoConnect },		// Passive
		  {nInput,		nPassive,		nInput,			nOutput,			nBiDir,				nPower,			nNoConnect },		// Input
		  {nOutput,		nOutput,		nOutput,		ERR_OUTPUT,			ERR_OUTPUTBIDIR,	ERR_POWERBIDIR,	nNoConnect },		// Output
		  {nBiDir,		nBiDir,			nBiDir,			ERR_OUTPUTBIDIR,	nBiDir,				ERR_POWERBIDIR,	nNoConnect },		// BiDir
		  {nPower,		nPower,			nPower,			ERR_OUTPUTTOPWR,	ERR_POWERBIDIR,		nPower,			nNoConnect },		// Power
		  {nNoConnect,	ERR_NOCONNECT,	ERR_NOCONNECT,	ERR_NOCONNECT,		ERR_NOCONNECT, 		ERR_NOCONNECT,	nNoConnect }		// NoConnect
};

void CTinyCadView::OnSpecialCheck()
{
	typedef std::map<CString,int> stringCollection;
	CString formattedBuffer;

	/// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	CDlgERCBox theDialog;
	static union { ErrorTest e; WORD i; } theErrorTest;

	theErrorTest.i = (WORD) CTinyCadRegistry::GetInt("ERC", 0xffff);

	/// Get the user's options
	theDialog.SetErrorTest(theErrorTest.e);
	if (theDialog.DoModal() != IDOK) {
		return;
	}

	theErrorTest.e = theDialog.GetErrorTest();  
	CTinyCadRegistry::Set("ERC",theErrorTest.i);

	/// Set the Busy icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	/// Generate the netlist
	CNetList netlist;
	netlist.m_follow_imports = false;
	CTinyCadMultiDoc *pDoc = static_cast<CTinyCadMultiDoc*>(GetDocument());
	netlist.MakeNet( pDoc );
	netCollection *nets = &netlist.m_nets;

	/// Delete all the errors which are currently in the design
	theERCListBox.Close();
	theERCListBox.Open( pDoc );
	int CurrentError = 0;

	/// Scan the design for unassigned references
	if ((theErrorTest.e).UnAssignedRefDes) {
		//TRACE("Scanning for unassigned reference designators...\n");
		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++) {
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd()) {
				CDrawingObject *pointer = *it;

				if (pointer->GetType()==xMethodEx3) {
					CString ref = static_cast<CDrawMethod *>(pointer)->GetField(CDrawMethod::Ref);
					//TRACE("  ==>Examining string \"%S\"\n",ref);
					if (ref.Find(_T('?'),0) != -1) {
						// We have an unassigned reference designator
						CString buffer;
						buffer.LoadString( ERR_UNASSIGNEDREFDES );
						formattedBuffer.Format(_T("%s:  [refdes=%s, page=\'%s\', xy=(%g,%g)]\n"),
							buffer, ref, pointer->m_pDesign->GetSheetName(), pointer->m_point_a.x/5, pointer->m_point_a.y/5);
						pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i),static_cast<CDrawMethod *>(pointer)->GetFieldPos(CDrawMethod::Ref),CurrentError++));
						theERCListBox.AddString(formattedBuffer);
						//TRACE("  ==>%S\n",buffer);
					}
				}
				++ it;
			}
		}
	}

	/// Scan the design for non-case distinct net names
	/// Scan the design for multiple net names on the same net
	if ((theErrorTest.e).MultipleNetNames || (theErrorTest.e).NonCaseDistinctNetNames) {
		typedef std::map<CString,nodeVector::iterator> nodeVectorCollection;
		nodeVectorCollection allNetNames;
	    //TRACE("Scanning for multiple net names and non-case distinct net names on the same net...\n");
		//int savedCurrentError = CurrentError;

		netCollection::iterator ni = nets->begin();
		while (ni != nets->end())
		{
			stringCollection netNames;	//Every net name assigned to this net will be collected in this temporary collection to help find multiple net names
			nodeVectorCollection netNameNodes;
			CString buffer;
			CString formattedBuffer;

			int net = (*ni).first;
			//TRACE("  ==>Scanning for net names contained in net=%d\n", net);

			nodeVector &v = (*ni).second;

			nodeVector::iterator vi = v.begin();	//Traverse the nodes in the netlist
			while (vi != v.end())
			{
				CString lcNetName;
				CString labelName;
				CNetListNode &node = *vi;

				//Net names are either an explicit label (xLabelEx2) or implied from a power pin (xPower)
				if (node.m_parent && (node.m_parent->GetType() == xLabelEx2)){
					labelName = static_cast<CDrawLabel*>(node.m_parent)->GetValue();
					if (netNames.find( labelName) == netNames.end()) {
						netNames[labelName] = net;	//This is a new net name label for this node - add it to the list
						netNameNodes[labelName] = vi;
						//TRACE("    ==>Object:  xLabelEx2=\"%S\" on net=%d added to the list of names for this net\n", static_cast<CDrawLabel*>(node.m_parent)->GetValue(), node.m_NetList);

						//if this net name is unique in this particular net, it should also be unique in the entire design
						//put this net name (in lower case) in the flattened net name list as well as a way to determine if it is
						//already in this design with some other combination of alphabet case
						lcNetName = labelName;
						lcNetName.MakeLower();
						if (allNetNames.find(lcNetName) == allNetNames.end()) {
							//This net name in lower case only is not yet in the list so put it in
							allNetNames[lcNetName] = vi;
							//TRACE("      Adding net name \"%S\" to allNetNames[] as \"%S\"\n",label_name, lcNetName);
						}
						else {
							//While this lower case version of the label_name should not have already been in this list, it is
							//so generate a warning that net names that are not case distinct are present in this design
							//TRACE("      Warning:  Net name \"%S\" is already in this design with a different character case and identifies a non-connected net\n",labelName);
							buffer.LoadString(ERR_NONDISTINCTNET);
							formattedBuffer.Format(_T("%s:  \"%s\", \"%s\""),buffer, labelName,*(allNetNames.find(lcNetName)));
							pDoc->GetSheet(node.m_sheet-1)->Add( new CDrawError(pDoc->GetSheet(node.m_sheet-1), node.m_a, CurrentError++));
							theERCListBox.AddString(formattedBuffer);
						}
					}
					else {
						//TRACE("    ==>Object:  xLabelEx2=\"%S\" on net=%d is already in the list of names for this net - ignoring\n", static_cast<CDrawLabel*>(node.m_parent)->GetValue(), node.m_NetList);
					}
				}
				else if (node.m_parent && (node.m_parent->GetType() == xPower)) {
					labelName = netlist.get_power_label((CDrawPower *) node.m_parent);

					if (netNames.find( labelName) == netNames.end()) {
						netNames[labelName] = net;	//This is a new net name label for this node - add it to the list
						netNameNodes[labelName] = vi;
						//TRACE("    ==>Object:  xPower=\"%S\" on m_net=%d added to the list of names for this net\n",powerLabel,node.m_NetList);

						//if this net name is unique in this particular net, it should also be unique in the entire design
						//put this net name (in lower case) in the flattened net name list as well as a way to determine if it is
						//already in this design with some other combination of alphabet case
						lcNetName = labelName;
						lcNetName.MakeLower();
						if (allNetNames.find(lcNetName) == allNetNames.end()) {
							//This net name in lower case only is not yet in the list so put it in
							allNetNames[lcNetName] = vi;
							//TRACE("      Adding net name \"%S\" to allNetNames[] as \"%S\"\n",powerLabel, lcNetName);
						}
						else {
							//While this lower case version of the label_name should not have already been in this list, it is
							//so generate a warning that net names that are not case distinct are present in this design
							//TRACE("      Warning:  Net name \"%S\" is already in this design with a different character case and identifies a non-connected net\n",labelName);
							buffer.LoadString(ERR_NONDISTINCTNET);
							formattedBuffer.Format(_T("%s:  \"%s\", \"%s\""),buffer, labelName,*(allNetNames.find(lcNetName)));
							pDoc->GetSheet(node.m_sheet-1)->Add( new CDrawError(pDoc->GetSheet(node.m_sheet-1), node.m_a, CurrentError++));
							theERCListBox.AddString(formattedBuffer);
						}
					}
					else {
						//TRACE("    ==>Object:  xPower=\"%S\" on m_net=%d is already in the list of names for this net - ignoring\n",powerLabel,node.m_NetList);
					}
				}

				++ vi;
			}

			if (netNames.size() > 1) {
				//Note:  A non-case distinct net name will always produce a multiple net name warning, unless multiple net name warnings are turned off
				if ((theErrorTest.e).MultipleNetNames) {	//record the multiple net name warnings
					//TRACE("    Warning:  Net node %d contains %d different net names\n",net, netNames.size());
					buffer.LoadString( ERR_MULTIPLENETNAMES );	//This is the base error message string
					//TRACE("    ==>Base msg=%S\n",buffer);

					//Now unpack and format the multiple net names
					nodeVectorCollection::iterator nv_it = netNameNodes.begin();
					CString stuff=(*nv_it).first;
					formattedBuffer.Format(_T("%s:  [\"%s\""), buffer, (*nv_it).first);	//Get the base message and the first net name
					//TRACE("       First formatted msg = %S\n", formattedBuffer);

					while (++nv_it != netNameNodes.end()) {
						buffer.Format(_T(",\"%s\""),(*nv_it).first);
						formattedBuffer += buffer;	//concatenate the next net name onto the end
						//TRACE("       Next formatted msg = %S\n", formattedBuffer);
					}
					formattedBuffer += ']';

					//Now unpack and identify each net name label that is a duplicate, using the error message string that contains all of the net names
					for (nv_it = netNameNodes.begin(); nv_it != netNameNodes.end(); nv_it++) {
						CNetListNode &node = *((*nv_it).second);
						pDoc->GetSheet(node.m_sheet-1)->Add( new CDrawError(pDoc->GetSheet(node.m_sheet-1), node.m_a, CurrentError++));
						theERCListBox.AddString(formattedBuffer);
						//TRACE("  ==>%S\n",formattedBuffer);
					}
				}
			}
			++ ni;
		}
		//TRACE("Multiple net name test found %d errors\n",CurrentError-savedCurrentError);
	}

	/// Scan the design for duplicated references
	if ((theErrorTest.e).DupRef)
	{
		std::set<CString>	refs;
		CString last = "";

		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++) {
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd()) {
				CDrawingObject *pointer = *it;

				if (pointer->GetType()==xMethodEx3) {
					CString ref = static_cast<CDrawMethod *>(pointer)->GetField(CDrawMethod::Ref);

					if (refs.find( ref ) != refs.end()) {
						// We have a duplicate...
						CString buffer;
						buffer.LoadString( ERR_DUPREF );
						formattedBuffer.Format(_T("%s:  [Object=\"%s\", RefDes=%s, Page #%d]\n"),buffer, pointer->GetName(), ref, i+1);
						pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i),static_cast<CDrawMethod *>(pointer)->GetFieldPos(CDrawMethod::Ref),CurrentError++));
						theERCListBox.AddString(formattedBuffer);
					}
					else {
						refs.insert( ref );
					}
				}

				++ it;
			}
		}
	}

	/// Scan netlist to determine the type of each object contained on each net.  Determine if the object type and the net type are compatible
	netCollection::iterator nit = nets->begin();
	while (nit != nets->end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();
		CString netObjectName;
		CString netObjectRefDes;
		CString netObjectSheetName;
		CString netObjectXY;

		int theNetType = nUnknown;
		CString lastPower = "";
		int connections = 0;

		CDPoint pos;
		int sheet = 0;

		while (theNetType < ERR_BASE && nv_it != (*nit).second.end()) 
		{
			CNetListNode& theNode = *nv_it;
			//CNetListNode& savedNode = theNode;

			CDrawingObject* pObject = theNode.m_parent;
			
			if (pObject != NULL)
			{
				//Keep a few identifying items around to help format intelligible error messages after the second object is found

				// Determine the type of this node
				int node_type = nUnknown;
				switch (pObject->GetType()) 
				{
				case xPower:
					node_type = nPower;
					if (lastPower=="")
					{
						lastPower = static_cast<CDrawPower *>(pObject)->GetValue();
					}
					else 
					{
						if ( lastPower != static_cast<CDrawPower *>(pObject)->GetValue() ) 
						{
							theNetType = ERR_POWER;
							node_type = nUnknown;
						}
					}
					// power symbols should not increment the number of connections
					pos = pObject->m_point_a;	//This will be used to locate this object on the sheet from the error message
					sheet = theNode.m_sheet;

					netObjectName.Format(_T("Obj=%s"), pObject->GetName());
					netObjectRefDes= "RefDes=N/A";
					netObjectSheetName.Format(_T("Sheet=#%d"),theNode.m_sheet);
					netObjectXY.Format(_T("xy=(%g,%g)"),theNode.m_a.x/5, theNode.m_a.y/5);
					break;
				case xNoConnect:
					node_type = 1;
					connections ++;
					pos = pObject->m_point_a;
					sheet = theNode.m_sheet;
					netObjectName.Format(_T("Obj=%s"), pObject->GetName());
					netObjectRefDes= "RefDes=N/A";
					netObjectSheetName.Format(_T("Sheet=#%d"),theNode.m_sheet);
					netObjectXY.Format(_T("xy=(%g,%g)"),theNode.m_a.x/5, theNode.m_a.y/5);
					break;
				case xPin:
				case xPinEx:
					{
						CDrawPin *pPin = static_cast<CDrawPin*>( pObject );
						switch(pPin->GetElec()) 
						{
						case 0:		// Input
							node_type = nInput; 
							break;
						case 1:		// Output
							node_type = nOutput; 
							break;
						case 2:		// Tristate
							node_type = nBiDir; 
							break;
						case 3:		// Open Collector
							node_type = nBiDir; 
							break;
						case 4:		// Passive
							node_type = nPassive; 
							break;
						case 5:		// Input/Output
							node_type = nBiDir; 
							break;
						case 6:		// Not Connected
							node_type = nNoConnect;
							break;
						}
						
						if (pPin->IsPower()) 
						{
							node_type = nPower;
						}

						pos = pPin->GetActivePoint(theNode.m_pMethod);
						sheet = theNode.m_sheet;
						connections ++;

					netObjectName.Format(_T("Obj=%s"), pObject->GetName());
						netObjectRefDes.Format(_T("RefDes=%s, Pin Number=%s, Pin Name=%s"),theNode.m_reference, pPin->GetNumber(), pPin->GetPinName());
						netObjectSheetName.Format(_T("Sheet=\'%s\'"),theNode.m_pMethod->m_pDesign->GetSheetName());
						netObjectXY.Format(_T("xy=(%g,%g)"),theNode.m_a.x/5, theNode.m_a.y/5);
					}
					break;
				}
				
				theNetType = (node_type!=nUnknown) ? ErcTable[theNetType][node_type] : theNetType;
			}

			++ nv_it;
		}

		int ErrorNumber = 0;

		if (connections == 1)
		{
			theNetType = ERR_UNCONNECT;
		}
	

		switch (theNetType) 
		{
			case nUnknown:
				if (connections > 0)
				{
					ErrorNumber = ERR_UNCONNECTED;
				}
				break;
			case nInput:
				ErrorNumber = ERR_NOUTPUT;
				break;
			case nNoConnect:
				if (connections > 2)
				{
					theNetType = ERR_UNCONNECT;
				}
				break;
			default:
				ErrorNumber = theNetType;
				break;
		}

		/// Is this error to be reported?
		switch (ErrorNumber) 
		{
		case ERR_UNCONNECT:
			if (!((theErrorTest.e).UnConnect))
				ErrorNumber = -1;
			break;
		case ERR_POWER:
			if (!((theErrorTest.e).Power))
				ErrorNumber = -1;
			break;
		case ERR_NOCONNECT:
			if (!((theErrorTest.e).NoConnect))
				ErrorNumber = -1;
			break;
		case ERR_NOUTPUT:
			if (!((theErrorTest.e).NoOutput))
				ErrorNumber = -1;
			break;
		case ERR_DUPREF:
			if (!((theErrorTest.e).DupRef))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUT:
			if (!((theErrorTest.e).Output))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUTTOPWR:
			if (!((theErrorTest.e).OutputPwr))
				ErrorNumber = -1;
			break;
		case ERR_UNCONNECTED:
			if (!((theErrorTest.e).UnConnected))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUTBIDIR:
			if (!((theErrorTest.e).Output))
				ErrorNumber = -1;
			break;
		case ERR_POWERBIDIR:
			if (!((theErrorTest.e).OutputPwr))
				ErrorNumber = -1;
			break;
		}

		if (ErrorNumber >= ERR_BASE) 
		{
			CString buffer;
			buffer.LoadString( ErrorNumber );
			formattedBuffer.Format(_T("%s:  [%s, %s, %s, %s]"),
				buffer,
				netObjectName, netObjectRefDes, netObjectSheetName, netObjectXY);
			pDoc->GetSheet(sheet-1)->Add(new CDrawError(pDoc->GetSheet(sheet-1),pos,CurrentError++));
			theERCListBox.AddString(formattedBuffer);
		}

		++ nit;
	}


	/// Were any errors detected?
	if (CurrentError == 0) 
	{
		CString buffer;
		buffer.LoadString( ERR_NOERROR );
		theERCListBox.AddString(buffer);
	}

	/// Set the normal icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );

	/// Re-Draw the window
	Invalidate();

}

