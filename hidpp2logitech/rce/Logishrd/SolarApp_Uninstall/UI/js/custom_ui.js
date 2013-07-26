/*

CUSTOM FORM ELEMENTS

Created by Ryan Fait
www.ryanfait.com

The only thing you need to change in this file is the following
variables: checkboxHeight, radioHeight and selectWidth.

Replace the first two numbers with the height of the checkbox and
radio button. The actual height of both the checkbox and radio
images should be 4 times the height of these two variables. The
selectWidth value should be the width of your select list image.

You may need to adjust your images a bit if there is a slight
vertical movement during the different stages of the button
activation.

Visit http://ryanfait.com/ for more information.

Modifications by Logitech Engineering Shared Services
05/11/09 - Bill Prescott Logitech Inc
    
	Modified the script to handle accelerators and input focus.  The focused input
	element will use a set of alternate images that represent the focused button
	states for radio buttons and checkboxes.  The select control is simply modified
	to have a dashed border.  This causes a slight expansion of the control a a bit
	of jumping of the control
*/

var checkboxHeight = 25;
var radioHeight = 25;
var selectWidth = 300;
var focusedCtrl=null;

/* No need to change anything after this */

document.write('<style type="text/css">input.styled { /*display: none;*/ position:absolute;top:-1000px;left:-1000px;} select.styled { position: relative; width: ' + selectWidth + 'px; opacity: 0; filter: alpha(opacity=0); z-index: 5; }</style>');
//
// Note: The addevent() function referenced here is located in the logi_code.js file
//
var Custom = {
	init: function() {
		var inputs = document.getElementsByTagName("input"), span = Array(), textnode, option, active;
		for(a = 0; a < inputs.length; a++) {
			if((inputs[a].type == "checkbox" || inputs[a].type == "radio") && inputs[a].className == "styled") {
				span[a] = document.createElement("span");
				span[a].className = inputs[a].type;

				if(inputs[a].checked == true) {
					if(inputs[a].type == "checkbox") {
						position = "0 -" + (checkboxHeight*2) + "px";
						span[a].style.backgroundPosition = position;
					} else {
						position = "0 -" + (radioHeight*2) + "px";
						span[a].style.backgroundPosition = position;
					}
				}
				inputs[a].parentNode.insertBefore(span[a], inputs[a]);
				inputs[a].onchange = Custom.clear;
				span[a].onmousedown = Custom.pushed;
				span[a].onmouseup = Custom.check;
				span[a].onclick = Custom.clickIt;
				document.onmouseup = Custom.clear;
				addEvent(inputs[a],'focus',Custom.focus);
				addEvent(inputs[a],'blur',Custom.blur);
				addEvent(inputs[a],'click',Custom.clear);  
			}
		}
		inputs = document.getElementsByTagName("select");
		for(a = 0; a < inputs.length; a++) {
			if(inputs[a].className == "styled") {
				option = inputs[a].getElementsByTagName("option");
				active = option[0].childNodes[0].nodeValue;
				textnode = document.createTextNode(active);
				for(b = 0; b < option.length; b++) {
					if(option[b].selected == true) {
						textnode = document.createTextNode(option[b].childNodes[0].nodeValue);
					}
				}
				span[a] = document.createElement("span");
				span[a].className = "select";
				span[a].id = "select" + inputs[a].name;
				span[a].appendChild(textnode);
				inputs[a].parentNode.insertBefore(span[a], inputs[a]);
				//inputs[a].onchange = inputs[a].onchange+Custom.choose;
				addEvent(inputs[a],'focus',Custom.focus);
				addEvent(inputs[a],'blur',Custom.blur);
				addEvent(inputs[a],'change',Custom.choose);
			}
		}
	},
	clickIt: function() {
		element = this.nextSibling;
		if(element.type=="radio")
		{
			element.click();
			focusedCtrl=element;	// probably not needed but best to be safe
		}
	},
	pushed: function() {
		element = this.nextSibling;
		
		if(element.checked == true && element.type == "checkbox") {
			var focusOffset=0;
			if (element==focusedCtrl||element.focused)
			{
				focusOffset=checkboxHeight*4;
			}
			var imageOffset=checkboxHeight*3+focusOffset;
			this.style.backgroundPosition = "0 -" + imageOffset + "px";
		} else if(element.checked == true && element.type == "radio") {
			this.style.backgroundPosition = "0 -" + radioHeight*6 + "px";
			focusedCtrl=element;
		} else if(element.checked != true && element.type == "checkbox") {
			var focusOffset=0;
			if (element==focusedCtrl||element.focused)
			{
				focusOffset=checkboxHeight*4;
			}
			var imageOffset=checkboxHeight+focusOffset;
			this.style.backgroundPosition = "0 -" + imageOffset + "px";
		} else {
			this.style.backgroundPosition = "0 -" + radioHeight*5 + "px";
			focusedCtrl=element;
		}
		element.focus();
	},
	check: function() {
		element = this.nextSibling;
		if(element.checked == true && element.type == "checkbox") {
			if (element!=focusedCtrl)
			{
				this.style.backgroundPosition = "0 0";
			}
			else
			{
				var imageOffset=checkboxHeight*5;
				this.style.backgroundPosition = "0 -" + imageOffset + "px";
			}
			element.checked = false;
		} else {
			if(element.type == "checkbox") {
			    var focusOffset=0;
				if (element==focusedCtrl)
				{
					focusOffset=checkboxHeight*4;
				}
				var imageOffset=checkboxHeight*2+focusOffset;
				this.style.backgroundPosition = "0 -" + imageOffset + "px";
			} else {
				this.style.backgroundPosition = "0 -" + radioHeight*7 + "px";
				group = this.nextSibling.name;
				inputs = document.getElementsByTagName("input");
				for(a = 0; a < inputs.length; a++) {
					if(inputs[a].name == group && inputs[a] != this.nextSibling) {
						inputs[a].previousSibling.style.backgroundPosition = "0 0";
					}
				}
				focusedCtrl=element;
			}
			element.checked = true;
		}
		element.focus();
	},
	clear: function() {
		inputs = document.getElementsByTagName("input");
		for(var b = 0; b < inputs.length; b++) {
			if(inputs[b].type == "checkbox" && inputs[b].checked == true && inputs[b].className == "styled") 
			{
				var focusOffset=0;
				if (inputs[b]==focusedCtrl)
				{
					focusOffset=checkboxHeight*4;
				}
				var imageOffset=checkboxHeight*2+focusOffset;
				inputs[b].previousSibling.style.backgroundPosition = "0 -" + imageOffset + "px";
			} 
			else if(inputs[b].type == "checkbox" && inputs[b].className == "styled") 
			{
				if (inputs[b]==focusedCtrl)
				{
					var imageOffset=checkboxHeight*4;
					inputs[b].previousSibling.style.backgroundPosition = "0 -" + imageOffset + "px";
				}
				else
				{
					inputs[b].previousSibling.style.backgroundPosition = "0 0";
				}
			} 
			else if(inputs[b].type == "radio" && inputs[b].checked == true && inputs[b].className == "styled") 
			{
				var focusOffset=0;
				if (inputs[b]==focusedCtrl)
				{
					focusOffset=radioHeight*4;
				}
				var imageOffset=radioHeight*2+focusOffset;
				inputs[b].previousSibling.style.backgroundPosition = "0 -" + imageOffset + "px";
			} 
			else if(inputs[b].type == "radio" && inputs[b].className == "styled") 
			{
				if (inputs[b]==focusedCtrl)
				{
					var imageOffset=radioHeight*4;
					inputs[b].previousSibling.style.backgroundPosition = "0 -" + imageOffset + "px";
				}
				else
				{
					inputs[b].previousSibling.style.backgroundPosition = "0 0";
				}
			}
		}
	},
	choose: function() {
		option = this.getElementsByTagName("option");
		for(d = 0; d < option.length; d++) {
			if(option[d].selected == true) {
				document.getElementById("select" + this.name).childNodes[0].nodeValue = option[d].childNodes[0].nodeValue;
			}
		}
	},
	focus: function() {
		focusedCtrl=this;
		this.focused=true;
		if (this.type == "checkbox") 
		{
			element = this.previousSibling;
			if (this.checked==true)
			{
				element.style.backgroundPosition = "0 -" + checkboxHeight*7 + "px";
			}
			else
			{
				element.style.backgroundPosition = "0 -" + checkboxHeight*4 + "px";
			}
		}
		else if (this.type == "radio" && this.className== "styled") 
		{
			element = this.previousSibling;
			element.style.backgroundPosition = "0 -" + radioHeight*7 + "px";
		}
		else  
		{
			document.getElementById("select" + this.name).style.border="#999999 1px dotted";
		}
	},
	blur: function() {

		focusedCtrl=null;
		this.focused=false;
		if (this.type == "checkbox" && this.className== "styled") 
		{
			if (this.checked == true)
			 {
				this.previousSibling.style.backgroundPosition = "0 -" + checkboxHeight*2 + "px";
			 }
			 else
			 {
				this.previousSibling.style.backgroundPosition = "0 0";
			 }
		}
		else if (this.type == "radio" && this.className== "styled") 
		{
			 if (this.checked == true)
			 {
				this.previousSibling.style.backgroundPosition = "0 -" + radioHeight*2 + "px";
			 }
			 else
			 {
				this.previousSibling.style.backgroundPosition = "0 0";
			 }
		}
		else if (this.className== "styled")
		{
			document.getElementById("select" + this.name).style.border="";
		}
	}
}
window.onload = Custom.init;