//
// Logitech Master Installer JavaScript Code
// Useful helper functions that use Localized String for installation projects
//
// Author: George Montemayor - Engineering Shared Services
// (C) 2009 Logitech - All Rights Reserved
// 


// Formats a numerical byte value into a localized string.
// The size is converted into KB or MB units if the size is >= 10KB or >= 1MB, respectively.
var KB_UNIT = 1000; //1024;
var id_secs = getString( "IDS_TIMEUNIT_SECS" );
var id_kbps = getString( "IDS_DOWNLOADSPEED_KBPS" );
var id_bps = getString( "IDS_DOWNLOADSPEED_BPS" );
var id_mbps = getString("IDS_DOWNLOADSPEED_MBPS");
var id_bytes = getString( "IDS_FILESIZE_BYTES" );
var id_kbytes = getString( "IDS_FILESIZE_KBYTES" );
var id_mbytes = getString( "IDS_FILESIZE_MBYTES" );


function FormatFilesize( size )
{
	if ( size < 10*KB_UNIT)	// under 10KB uses bytes unit
		return size + " " + getString( "IDS_FILESIZE_BYTES" );

	if ( size < KB_UNIT*KB_UNIT )	// under 1MB uses KB unit
	    return new Number(size / KB_UNIT).toFixed(0) + " " + id_kbytes;

	return new Number(size / (KB_UNIT * KB_UNIT)).toFixed(0) + " " + id_mbytes;
}

// Formats the time value (unit: seconds) into a localized string.
function FormatTime( secs )
{
 	if ( isNaN(secs) == true )
 	    return " ";
    var hours = Math.floor(secs / (60 * 60));
    var divisor_for_minutes = secs % (60 * 60);
    var minutes = Math.floor(divisor_for_minutes / 60);
    var divisor_for_seconds = divisor_for_minutes % 60;
    var seconds = Math.ceil(divisor_for_seconds);

    var time;
    if (hours > 0)
        time = hours + ":" + zeroPad(minutes, 2) + ":" + zeroPad(seconds, 2);
    else if (minutes > 0)
        time = zeroPad(minutes, 2) + ":" + zeroPad(seconds, 2);
    else
        time = seconds + " " + id_secs;
    return time;
}
function zeroPad(num, count) {
    var numZeropad = num + '';
    while (numZeropad.length < count) {
        numZeropad = "0" + numZeropad;
    }
    return numZeropad;
}     

// Formats a numerical byte value into a localized string.
// The size is converted into KBPS or MBPS units if the size is >= 1KB or >= 1MB, respectively.
function FormatSpeed( size )
{
	if ( size < 1*KB_UNIT)	// under 1KB uses bytes unit
	    return size + " " + id_bps;

	if ( size < KB_UNIT*KB_UNIT )	// under 1MB uses KB unit
	    return new Number(size / KB_UNIT).toFixed(0) + " " + id_kbps;

	return new Number(size / (KB_UNIT * KB_UNIT)).toFixed(0) + " " + id_mbps;
}
