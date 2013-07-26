//
// Logitech Master Installer JavaScript Code
// Useful helper functions for installation projects
//
// Author: Bill Prescott - Engineering Shared Services
// (C) 2009 Logitech - All Rights Reserved
// 
// addEvent - Adds an event handler to an event.  Allows event extension
// obj - The object to add the event to
// type - The type of event (click, focus, etc.)
// fn - The function to attach
//
function addEvent( obj, type, fn ) {
	if ( obj.attachEvent ) {
	obj['e'+type+fn] = fn;
	obj[type+fn] = function(){obj['e'+type+fn]( window.event );}
	obj.attachEvent( 'on'+type, obj[type+fn] );
	} else
	obj.addEventListener( type, fn, false );
}

//
// This function will display the language code in the lower right of the screen. 
// Helpful during development and before the localization is complete
//
var showLangCode = true;
function displayLangCode()
{
	if (showLangCode)
	{
		var langCode = window.external.EvaluateExpression('langcode');
		document.write("<div style='font-size:8px;font-weight:bold;position:absolute;top:520px;left:676px;'>"+langCode+"</div>");
	}
}
//
// Defines a language specific style sheet.  It will get the current language from the master installer and then add the style sheet to the 
// HTML document
//
function getStyles()
{
	var langCode = window.external.EvaluateExpression('langcode');
	document.write("<link rel='stylesheet' href='../"+langCode+"/wizpage.css' type='text/css' media='screen'>");
}

//
// This function will replace or set the strings for an HTML page by pulling string IDs from the Master Installer
// and replace the inner HTML for the defined element.  
// The function will also sets the shortcut keystroke and adds the underscrore character to the string.
//
//  This is how it works: The code recognizes two tags string='ID' and accel='ID'.  The string='ID' causes two things to happen,
//  first the string is read from the master installer and used to replace the inner HTML.  Then an accellerator is looked up that
//  has the form STRING_ID_ACCEL.  If this exists it is read and then the accelerator key is highlighted using the underscore character.
//  Finally any accell='ID' parameter causes the accelerator to be looked up and the character is used accesskey property for the specified 
//  element.  This tactic allows the text and active elements to be tied but disconnected.
//  Example: <a href="#" accel="IDS_BTN_QUIT" tabindex="3" onclick="this.click();return false;" class="button primary" id="btnCancel"><b string="IDS_BTN_QUIT"></b></a>
//
// Note: The HTML body must have the id='theBody' for this function to correcly traverse the DOM
//
function loadStrings()
{
	var elem=document.getElementById('theBody');
	if (elem)
	{
	  var inputs = elem.all;
	  if (inputs)
	  {
		for(var i = 0; i < inputs.length; i++) 
		{
			if (inputs[i].string)
			{
				// First convert to the numeric ID from the #IDS_ symbolic value
				var theString = window.external.EvaluateExpression('#'+inputs[i].string);
				//window.external.Log('the IDS retuned '+theString);
				if (theString != inputs[i].string)
				{
					// Use the LoadString function to do the actual replacement
					var exprString='LoadString("'+theString+'")';
					var accelIdString = window.external.EvaluateExpression('#'+inputs[i].string+'_ACCEL');
					theString = window.external.EvaluateExpression(exprString);
					// Do accelerator if it exists
					if (accelIdString!=inputs[i].string+'_ACCEL')
					{
						var accelString='LoadString("'+accelIdString+'")';
						theAccel = window.external.EvaluateExpression(accelString);
						theString=theString.replace(theAccel,'<u>'+Left(theAccel,1)+'</u>'+Right(theAccel,theAccel.length-1));
					}
					inputs[i].innerHTML=theString;
					//window.external.Log('replacing '+inputs[i].string+' with '+theString);
				}
			}
			if (inputs[i].accel)
			{
			
				var theString = window.external.EvaluateExpression('#'+inputs[i].accel+'_ACCEL');
				if (theString != inputs[i].string+'_ACCEL')
				{
					var accelExpr='LoadString("'+theString+'")';
					theString = window.external.EvaluateExpression(accelExpr);
					//window.external.Log('the Accel key is '+theString);
					inputs[i].accessKey=Left(theString,1);
				}
			}
		}
	  }
	}
}

function getString(stringId)
{
	var theString = window.external.EvaluateExpression('#'+stringId);
	window.external.Log('the IDS retuned '+theString);
	if (theString != stringId)
	{
		// Use the LoadString function to do the actual replacement
		var exprString='LoadString("'+theString+'")';
		theString = window.external.EvaluateExpression(exprString);
		window.external.Log('the IDS retuned '+theString);
	}
	return(theString);
}

function loadString(stringId,elem)
{
	var theString = getString(stringId);
	if (elem)
	{
		elem.innerHTML=theString;
	}
}
//
// Simple function to return the left part of a string
//
function Left(str, n){
	if (n <= 0)
	    return "";
	else if (n > String(str).length)
	    return str;
	else
	    return String(str).substring(0,n);
}
//
// Simple function to return the right part of a string
//
function Right(str, n){
    if (n <= 0)
       return "";
    else if (n > String(str).length)
       return str;
    else {
       var iLen = String(str).length;
       return String(str).substring(iLen, iLen - n);
    }
}
//
// This function makes it easy to set the installation step.  It coordinates the
// CSS decoration with the current step for a page. Pass in the step (1 - n)
// The install engineer must make certain the defined step actually exists.
//
function setStep(step)
{
	for (var i=1; i<=step; i++)
	{
		var obj=document.getElementById('stepNum_'+i);
		if (obj)
		{
			if (i==step)
			{
				obj.className="stepNumSelected";
			}
			else
			{
				obj.className="stepNumDone";
				obj.innerHTML="";
			}
		}
		var obj=document.getElementById('stepText_'+i);
		if (obj)
		{
			if (i==step)
			{
				obj.className="stepTextSelected";
			}
			else
			{
				obj.className="stepTextDone";
			}
		}
	}
}
//
// This function will append an HTML document to the current one that is open
// Pass in the URL for the file you want to append
//
function appendHTML(url)
{
	var request = new ActiveXObject("Microsoft.XMLHTTP");
	request.open("GET", url);
	request.send(null);
	
	document.write(request.responseText);
}
//
// This function is used to read a URL and return the response code (good for checking before appending)
function readUrl(url)
{
	var request = new ActiveXObject("Microsoft.XMLHTTP");
	request.open("GET", url);
	request.send(null);

	return request.responseText;
}
//
// This function makes a DWORD for passage to Windows API functions
//
function MAKEDWORD(hi, lo)
{
	var dword = hi << 16;
	dword += lo;
	return dword;
}
//
// This function sets the inital focus to the specified element.  This is useful for installs that
// support keyboard based installs.  The function also sets the default focus element that is selected
// when a page is first opened.
//
var sfAttach = false;
var sfObj = null;
var sfTimeout = 0;
function selectFirst(obj, nTimeout)
{
	if (sfAttach == false)
	{
		sfAttach = true;
		sfTimeout = nTimeout;
		sfObj = obj;
		window.attachEvent("onfocus", selectFirst);
	}
	if (sfObj != null)
	{
		setTimeout(sfObj.id+".focus()", sfTimeout);
	}
}