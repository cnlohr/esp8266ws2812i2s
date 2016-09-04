//Copyright (C) 2015 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.

function initLEDs() {
var menItm = `
	<tr><td width=1><input type=submit onclick="ShowHideEvent( 'LEDs' ); KickLEDs();" value="LEDs"></td><td>
	<div id=LEDs class="collapsible">
	<table width=100% border=1><tr><td id=LEDCanvasHolder><CANVAS id=LEDCanvas width=512 height=100></CANVAS></td>
	<td><input type=button onclick="ToggleLEDPause();" id=LEDPauseButton value="|| / >"></td></tr></table>
	Select LEDs/Pattern: <input type=text id=LEDSelect value=1-4>  <br>
	#LEDs: <input type=number min=0 value=4 id=LEDNum style="width:5.5em;">
	<input type=color id=LEDColor> <input type=button value="Set Color" id=LEDCbtn> <input type=button value="Set Pattern" id=LEDPbtn> <input type=button value=Stop id=LEDSbtn> <input type=button value=Off id=LEDObtn>
	<p style="font-size:70%;font-style: italic">
	Patterns can include individual LEDs or ranges of LEDs separated by commata, e.g. "1-3,7,20-35".
	You can select colors via the color-picker or by holding <kbd>Alt</kbd> and clicking a color inside the color display.
	Alternatively, colors can be appended to a range, e.g. "1-3#ffff00". Rages without an appended color use the last specified one.
	Ranges can be entered by hand or selected in the color-display with the usual <kbd>Shift</kbd>+<kbd>LeftClick</kbd> and <kbd>Alt</kbd>+<kbd>LeftClick</kbd> combinations.
	If you set a pattern, use a low positive integer.
	"#LEDs" is the total number of LEDs connected to the esp8266 and must be provided.</p>
	</div>
	</td></tr>
`;
$('#MainMenu > tbody:last').after( menItm );
KickLEDs();
}

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


function strToByte(str, pos) {
	return parseInt(str.substr(pos, 2), 16);
}


function validateRange() {
	var obj = $('#LEDSelect');
	var val = obj.val();
	if( ! val.match(/^\d+(-\d+)?(#[a-zA-Z0-9]{6,6})?(,\d+(-\d+)?(#[a-zA-Z0-9]{6,6})?)*$/) ) {
	    obj.css( "background-color", "#ff0000");
	    return false;
	}
	obj.css( "background-color", "#ffffff");
	return val;
}


function SetPattern( ptrn ) {
	if( ! ptrn.match(/^\d+$/) ) {
		$('#LEDSelect').css( "background-color", "#ff0000");
		return false;
	}
	var numLEDs = parseInt($('#LEDNum').val());
	var color = document.getElementById('LEDColor').value;
	color = color.replace('#','');
	$('#LEDSelect').css( "background-color", "#ffffff");
	var qDat = new Uint8Array(ptrn==0 ? 8 : 5);
	var byte = 0;
	qDat[byte++] = "C".charCodeAt();
	qDat[byte++] = "P".charCodeAt();
	qDat[byte++] = parseInt(ptrn);
	qDat[byte++] = numLEDs>>8;
	qDat[byte++] = numLEDs%256;
	if( ptrn==0 ) {
		qDat[byte++] = strToByte(color, 0);
		qDat[byte++] = strToByte(color, 2);
		qDat[byte++] = strToByte(color, 4);
	}
	QueueOperation( qDat );
	return true;
}


// Mostly setup stuff
function KickLEDs()
{
	// Validate Number of LEDS
	$('#LEDNum').change( function(e) {
		nled = parseInt( $('#LEDNum').val() );
		if( val<0 || val>512 || isNaN(val) ) nled = 4;
		$('#LEDNum').val(nled);
	});

	// Select a pattern to be used continously
	$('#LEDPbtn').click( function(e) {
		var ptrn = $('#LEDSelect').val();
		return SetPattern( ptrn );
	});

	// Stop LEDs from changing
	$('#LEDSbtn').click( function(e) {
		return SetPattern( '255' );
	});

	// Turn all LEDs to black
	$('#LEDObtn').click( function(e) {
		document.getElementById('LEDColor').value = '#000000';
		return SetPattern( '0' );
	});

	$('#LEDColor').change( function(e){
		var sel = $('#LEDSelect');
		val = sel.val();
		sel.val( val.replace( /#[a-zA-Z0-9]{6,6}(?!.*#[a-zA-Z0-9]{6,6})/, this.value) );
		//sel.val( val.replace( /#\d{6,6}$/, this.color) );
	});

	// Color Picker (from Canvas)
	//$('#LEDCanvas').mousemove(function(e) {
	$('#LEDCanvas').click( function(e) {
	    var s = e.shiftKey, a = e.altKey, c = e.ctrlKey;
		var sel = $('#LEDSelect');
	    var pos = findPos(this);
	    var ctx = this.getContext('2d');
	    var x = e.pageX-pos.x;
	    var p = ctx.getImageData(x, e.pageY-pos.y, 1, 1).data;
	    var n = parseInt(x * nled / this.clientWidth)+1;
	    var clr = "#" + ("000000" + rgbToHex(p[0], p[1], p[2])).slice(-6)
	    document.getElementById('LEDColor').value = clr;
	    var val = validateRange();
	    //if( !val && !s ) return false;
	    var ptrns = val.split(',');
	    var endptrn = ptrns[ptrns.length-1]
	    var endclr = endptrn.split('#');
	    endclr = endclr[1] ? endclr[1] : false;
	    var singlet = endptrn.match(/^\d+$/);
	    if( a ) {
	    	; // Do nothing
	    } else if( !s && !c ) {
	    	sel.val(n);
	    } else if( s && !endclr ) {
	    	sel.val( val + '-' + n + clr );
	    } else if( c && !s && (endclr || singlet) ) {
	    	sel.val( (val ? val+',' : '') + n );
	    }
	    validateRange();
	});

	// Set LED color via Button
	$('#LEDCbtn').click( function(e) {
		var val = validateRange();
		if( !val ) return false;
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

		var qDat = new Uint8Array(3*numLEDs+3);
		var byte = 0;
		qDat[byte++] = "C".charCodeAt();
		qDat[byte++] = "T".charCodeAt();
		qDat[byte++] = " ".charCodeAt();
		for(var i=1; i<=numLEDs; ++i) {
			if( leds[i] && typeof(leds[i])!='undefined' ) {
				qDat[byte++] = strToByte(leds[i], 2); //String.fromCharCode(rVal);
				qDat[byte++] = strToByte(leds[i], 0);
				qDat[byte++] = strToByte(leds[i], 4);
			} else {
				var s = (i-1)*6;
				qDat[byte++] = strToByte(led_data, s+0);
				qDat[byte++] = strToByte(led_data, s+2);
				qDat[byte++] = strToByte(led_data, s+4);
			}
		}
		QueueOperation( qDat );
		return true;
	});

	$( "#LEDPauseButton" ).css( "background-color", (is_leds_running&&!pause_led)?"green":"red" );

	if( !is_leds_running && !pause_led )
		LEDDataTicker();
}


window.addEventListener("load", initLEDs, false);


function ToggleLEDPause()
{
	pause_led = !pause_led;
	KickLEDs();
}


// Update the display canvas
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
