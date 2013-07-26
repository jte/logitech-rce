///////////////////////////////////////////////////////////////////////////////
///*----------------------------------------------------------------------------
///
///  Copyright (c) 1999-2009 Logitech, Inc.  All Rights Reserved
///
///  This program is a trade secret of LOGITECH, and it is not to be reproduced,
///  published, disclosed to others, copied, adapted, distributed or displayed
///  without the prior authorization of LOGITECH.
///
///  Licensee agrees to attach or embbed this notice on all copies of the program,
///  including partial copies or modified versions thereof.
///
///  Description:
///
///     Package Uninstaller U/I Support code
///
//--------------------------------------------------------------------------*/
///  logi_uninstall.hs
///  
///
///
///  @author Bill Prescott @date 8/5/2009
///////////////////////////////////////////////////////////////////////////////
var GUID=window.external.EvaluateExpression("GUID");
var linkedItems = [];
var activeItems = [];
var numShown=0;
var numFound=0;
var panelTitle;
var appTitle;
var exitTitle;
var productName;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This function constructs a Logi checkbox name.  It handles the transition
// and build up of names from 1-9 and 10 onward as two cases since there is no
// real formatter in JS.
//
function getCheckboxName(iNum)
{
	var ctrlNum;
	if (iNum<10)
	{
		ctrlNum='Logi_checkbox_0'+iNum;
	}
	else
	{
		ctrlNum='Logi_checkbox_'+iNum;
	}
	return ctrlNum;
}
//
// This function creates a single <tr> line for the component
//
function makeComponentLine(iNum)
{
    // Improvement needed: currently I am unable to pass the icon name into the tr block as a variable, so I have to have a separate tr line for
    // each item in the list.
	var ctrlNum=getCheckboxName(iNum);
	var showDisplay="";
	var pngFile="";

	var expr='RegVal("HKEY_LOCAL_MACHINE\\Software\\Logitech\\Sn1","RunningApp'+iNum+'","")';
  var EnableIE=window.external.EvaluateExpression(expr);
  if (EnableIE == "")
    showDisplay="none";
    
    if (iNum == 1)
    {
      pngFile="../images/icons/IE.png";
    }
    if (iNum == 2)
    {
      pngFile="../images/icons/ffx.png";
    }
    if (iNum == 3)
    {
      pngFile="../images/icons/chr.png";
    }
 
    var tempString="";
   	tempString='<tr id="c'+iNum+'_line" style="display:none;"><td colspan="3"></td></tr><tr id="c'+iNum+'_item" style="display:'+showDisplay+';"><td style="width:10%;padding:1px 1px 1px 10px"><img id="picon'+iNum+'" src="'+pngFile+'"/></td><td id="c'+iNum+'_name" style="width:70%; padding:10px 1px 10px 1px">'+EnableIE+'</td></tr>';

	return(tempString);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This function dynamically builds the component table based on the maximum number of list items passed in
// It saves a lot of repeated code and possible typos.
function makeComponentTable(maxItems)
{
	var i;
	var table='<table width="96%" cellpadding="0" cellspacing="0" id="components">';
	for (i=1;i<=maxItems;i++)
	{
		table=table+makeComponentLine(i);
	}
	table=table+'</table>';
	return(table);
}
//
// This function gets specific U/I element from the registry and handles the replacement on the HTML layout.
function GetCustomStrings()
{
	var valList = [new strContainer('ExitTitle','sidebarTitleText'),new strContainer('FrameTitle','panelTitle'),new strContainer('PanelTitle','panelTitle'),new strContainer('ProductName','sidebarTitleText')];
	for (i=0;i<valList.length;i++)
	{
		var expr='RegVal("HKEY_LOCAL_MACHINE\\Software\\Logitech\\Installer\\'+GUID+'","'+valList[i].regVal+'","",1)';
		//alert(expr);
		var stringVal=window.external.EvaluateExpression(expr);
		if (stringVal != "")
		{
			var elem=document.getElementById(valList[i].idVal)
			if (elem)
			{
				elem.innerHTML=stringVal;
			}
		}
	}
	//window.external.SetString('IDS_TITLE','Logitech Setpoint Uninstaller');
	//window.external.SetString('IDS_ABORTDLG_TITLE','Logitech Setpoint Uninstaller');
}

function strContainer(regVal,idVal)
{
	this.regVal=regVal;
	this.idVal=idVal;
}

