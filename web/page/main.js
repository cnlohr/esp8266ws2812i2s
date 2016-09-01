//Copyright (C) 2015 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.

is_leds_running = false;
pause_led = false;
led_data = [];
nled = 4;


function findPos(obj) {
    var curleft = 0, curtop = 0;
    if (obj.offsetParent) {
        do {
            curleft += obj.offsetLeft;
            curtop += obj.offsetTop;
        } while (obj = obj.offsetParent);
        return { x: curleft, y: curtop };
    }
    return undefined;
}


function rgbToHex(r, g, b) {
    if (r > 255 || g > 255 || b > 255)
        throw "Invalid color component";
    return ((r << 16) | (g << 8) | b).toString(16);
}


function KickLEDs()
{
	// Validate Number of LEDS
	$('#LEDNum').change( function(e) {
		var val = parseInt( $('#LEDNum').val() );
		if( val<0 || val>512 || isNaN(val) ) $('#LEDNum').val(4);
	});

	// Color Picker (from Canvas)
	//$('#LEDCanvas').mousemove(function(e) {
	$('#LEDCanvas').click( function(e) {
	    var pos = findPos(this);
	    var x = e.pageX - pos.x;
	    var y = e.pageY - pos.y;
	    var coord = "x=" + x + ", y=" + y;
	    var c = this.getContext('2d');
	    var p = c.getImageData(x, y, 1, 1).data;
	    hex = "#" + ("000000" + rgbToHex(p[0], p[1], p[2])).slice(-6);
	    document.getElementById('LEDColor').value = hex;
	});

	// Set LED color via Button
	$('#LEDCbtn').click( function(e) {
		var val = $('#LEDSelect').val();
		if( ! val.match(/^\d+(-\d+)?(#[a-zA-Z0-9]{6,6})?(,\d+(-\d+)?(#[a-zA-Z0-9]{6,6})?)*$/) ) {
		    $('#LEDSelect').css( "background-color", "#ff0000");
		    return false;
		}
		$('#LEDSelect').css( "background-color", "#ffffff");
		var numLEDs = parseInt( $('#LEDNum').val() );
		var toks = val.split(',');
		var leds = new Array();
		var color = document.getElementById('LEDColor').value;
		color = color.replace('#','');
		for(var i=0; i<toks.length; ++i) {
		    var colsplt = toks[i].split("#");
		    var rangesplt = colsplt[0].split("-");
		    if( colsplt.length > 1 ) color = colsplt[1];
		    if( rangesplt.length < 2 ) {
		        var idx = parseInt(rangesplt[0]);
		        if(idx<=numLEDs) leds[idx] = color;
		    } else {
		        var min = parseInt(rangesplt[0]);
		        var max = parseInt(rangesplt[1]);
		        if( min > max ) { var tmp=min; min=max; max=tmp; }
		        for(var j=min; j<=max && j<=numLEDs; ++j)
		            leds[j] = color;
		    }
		}

		var qStr = new Uint8Array(3*numLEDs+3);
		var byte = 0;
		qStr[byte++] = "C".charCodeAt(); qStr[byte++] = "T".charCodeAt();
		qStr[byte++] = " ".charCodeAt();
		for(var i=1; i<=numLEDs; ++i) {
			if( leds[i] && typeof(leds[i])!='undefined' ) {
				var rVal = parseInt(leds[i].substr(0, 2), 16);
				var gVal = parseInt(leds[i].substr(2, 2), 16);
				var bVal = parseInt(leds[i].substr(4, 2), 16);
				qStr[byte++] = gVal; //String.fromCharCode(rVal);
				qStr[byte++] = rVal;
				qStr[byte++] = bVal;
			} else {
				qStr[byte++] = parseInt(led_data.substr((i-1)*6+0, 2), 16);
				qStr[byte++] = parseInt(led_data.substr((i-1)*6+2, 2), 16);
				qStr[byte++] = parseInt(led_data.substr((i-1)*6+4, 2), 16);
			}
		} // console.log(leds);	console.log(qStr);
		QueueOperation( qStr );
		return true;
	});

	// Select a pattern to be used continously
	$('#LEDPbtn').click( function(e) {
		var ptrn = $('#LEDSelect').val();
		var numLEDs = parseInt($('#LEDNum').val());
		if( ! ptrn.match(/^\d+$/) ) {
			$('#LEDSelect').css( "background-color", "#ff0000");
			return false;
		}
		var color = document.getElementById('LEDColor').value;
		color = color.replace('#','');
		$('#LEDSelect').css( "background-color", "#ffffff");
		var qStr = new Uint8Array(ptrn==0 ? 8 : 5);
		var byte = 0;
		qStr[byte++] = "C".charCodeAt();
		qStr[byte++] = "P".charCodeAt();
		qStr[byte++] = parseInt(ptrn);
		qStr[byte++] = numLEDs>>8;
		qStr[byte++] = numLEDs%256;
		if( ptrn==0 ) {
			qStr[byte++] = parseInt(color.substr(0,2), 16);
			qStr[byte++] = parseInt(color.substr(2,2), 16);
			qStr[byte++] = parseInt(color.substr(4,2), 16);
		}
		QueueOperation( qStr );
		return true;
	});

	$( "#LEDPauseButton" ).css( "background-color", (is_leds_running&&!pause_led)?"green":"red" );

	if( !is_leds_running && !pause_led )
		LEDDataTicker();
}


window.addEventListener("load", KickLEDs, false);


function ToggleLEDPause()
{
	pause_led = !pause_led;
	KickLEDs();
}


function GotLED(req,data)
{
	var ls = document.getElementById('LEDCanvasHolder');
	var canvas = document.getElementById('LEDCanvas');
	var ctx = canvas.getContext('2d');
	if( canvas.width != ls.clientWidth-10 )   canvas.width = ls.clientWidth-10;
	if( ctx.canvas.width != canvas.clientWidth )   ctx.canvas.width = canvas.clientWidth;

	var secs = data.split( ":" );

	$( "#LEDPauseButton" ).css( "background-color", "green" );

	nled = Number( secs[1] );
	led_data = secs[2];
	var lastsamp = parseInt( led_data.substr(0,4),16 );
	ctx.clearRect( 0, 0, canvas.width, canvas.height );

	for( var i = 0; i < nled; i++ ) {
		var x2 = i * canvas.clientWidth / nled;
		var samp = led_data.substr(i*6,6);

		ctx.fillStyle = "#" + samp.substr( 2, 2 ) + samp.substr( 0, 2 ) + samp.substr( 4, 2 );
		ctx.lineWidth = 0;
		ctx.fillRect( x2, 0, canvas.clientWidth / nled+1, canvas.clientHeight );
	}

	LEDDataTicker();
}


function LEDDataTicker()
{
	if( IsTabOpen('LEDs') && !pause_led ) {
		is_leds_running = true;
		QueueOperation( "CL",  GotLED );
	} else is_leds_running = 0;
	$( "#LEDPauseButton" ).css( "background-color", (is_leds_running&&!pause_led)?"green":"red" );

}
