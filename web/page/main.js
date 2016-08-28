//Copyright (C) 2015 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.


is_leds_running = false;
pause_led = false;

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
		console.log(val);
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
		if( ! val.match(/^\d+(-\d+)?(,\d+(-\d+)?)*$/) ) {
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
			var range = toks[i].split("-");
			if( range.length == 1 ) {
				var idx = parseInt(toks[i]);
				if(idx<=numLEDs) leds[idx] = color;
			} else {
				var min = parseInt(range[0]);
				var max = parseInt(range[1]);
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
				qStr[byte++] = 0; qStr[byte++] = 0; qStr[byte++] = 1;
			}
		}
		console.log(leds);
		console.log(qStr);
		QueueOperation( qStr );
		return true;
	});

	// Select a pattern to be used continously
	$('#LEDPbtn').click( function(e) {
		var val = $('#LEDSelect').val();
		var numLEDs = $('#LEDNum').val();
		if( ! val.match(/^\d+$/) ) {
			$('#LEDSelect').css( "background-color", "#ff0000");
			return false;
		}
		$('#LEDSelect').css( "background-color", "#ffffff");
		QueueOperation( "CP\t" + val + "\t" + numLEDs );
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
	var h = ls.height;
	var w = ls.width;
	if( canvas.width != ls.clientWidth-10 )   canvas.width = ls.clientWidth-10;
	if( ctx.canvas.width != canvas.clientWidth )   ctx.canvas.width = canvas.clientWidth;

	var secs = data.split( ":" );

	$( "#LEDPauseButton" ).css( "background-color", "green" );

	var samps = Number( secs[1] );
	var data = secs[2];
	var lastsamp = parseInt( data.substr(0,4),16 );
	ctx.clearRect( 0, 0, canvas.width, canvas.height );

	for( var i = 0; i < samps; i++ ) {
		var x2 = i * canvas.clientWidth / samps;
		var samp = data.substr(i*6,6);
		var y2 = ( 1.-samp / 2047 ) * canvas.clientHeight;

		ctx.fillStyle = "#" + samp.substr( 2, 2 ) + samp.substr( 0, 2 ) + samp.substr( 4, 2 );
		ctx.lineWidth = 0;
		ctx.fillRect( x2, 0, canvas.clientWidth / samps+1, canvas.clientHeight );
	}

	var samp = parseInt( data.substr(i*2,2),16 );

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




