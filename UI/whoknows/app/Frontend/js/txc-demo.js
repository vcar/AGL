// parse location to get security token
var urlParams={};
location.search.substr(1).split("&").forEach(function(item) {
	var k = item.split("=")[0];
	var v = decodeURIComponent(item.split("=")[1]); 
	if (k in urlParams) urlParams[k].push(v); else urlParams[k] = [v];
});

var afb = new AFB("api"/*root*/, urlParams.token[0]);
var ws;
var curLat,prvLat;
var curLon,prvLon;
var vspeed = 0, espeed = 0, torque = 0;
var con,cons,consa = [ ];
var wdgVsp, wdgEsp, wdgTrq;

var cmp=0;
var esp_tmp,vsp_tmp,toque_tmp;

    var a = "hello world";
function myFunction(name,a) {
       $(document).ready(function(){
                $.ajax({
                  type: "GET",
                  url: 'http://192.168.1.108:5000/api',
                  data: { 'name':name,
			  'a':a
                        },
                  success: function(response){
                        
                      if(response!='-1' && response!='-2')
                          {
                              hide();
                              
                          }
                      
                  }
                });  
              
          });

    
}




function gotltftb(obj) {
	myFunction("Long_Term_Fuel_Trim_Bank1",obj.data.value);
}

function gotiap(obj) {
	myFunction("Intake_air_pressure",obj.data.value);
}

function gotapv(obj) {
	myFunction("Accelerator_Pedal_value",obj.data.value);
}

function gotfc(obj) {
	myFunction("Fuel_consumption",obj.data.value);
}

function gottof(obj) {
	myFunction("Torque_of_friction",obj.data.value);
}

function gotmiet(obj) {
	myFunction("Maximum_indicated_engine_torque",obj.data.value);
}

function gotet(obj) {
	myFunction("Engine_torque",obj.data.value);
}

function gotclv(obj) {
	myFunction("Calculated_LOAD_value",obj.data.value);
}

function gotaoac(obj) {
	myFunction("Activation_of_Air_compressor",obj.data.value);
}

function gotect(obj) {
	myFunction("Engine_coolant_temperature",obj.data.value);
}

function gottot(obj) {
	myFunction("Transmission_oil_temperature",obj.data.value);
}

function gotwvflh(obj) {
	myFunction("Wheel_velocity_front_left-hand",obj.data.value);
}

function gotwvfrh(obj) {
	myFunction("Wheel_velocity_front_right-hand",obj.data.value);
}

function gotwvrlh(obj) {
	myFunction("Wheel_velocity_rear_left-hand",obj.data.value);
}

function gottcs(obj) {
	myFunction("Torque_converter_speed",obj.data.value);
}




function gotStart(obj) {
	document.body.className = "started";
	vspeed = 0;
	espeed = 0;
	cons = undefined;
	consa = [ ];

}

function gotStop(obj) {
	document.body.className = "connected";
}

var msgcnt=0;
var msgprv=0;
var msgprvts=0;
function gotAny(obj) { 
	if (obj.event != "txc/STOP") {
		document.body.className = "started";
	}
	msgcnt++;
}

function updateMsgRate() {
	var now=+new Date();
	if (msgprvts) {
		var dt=now-msgprvts;
		msgrate=Math.round((msgcnt-msgprv)*10000/dt)/10;
		wdgMsg.innerHTML=String(msgrate);
	}

	msgprv=msgcnt;
	msgprvts=now;
}


function gotStat(obj) {
	wdgStat.innerHTML = obj.data;
}

function onAbort() {
	document.body.className = "not-connected";
}

function onOpen() {
	ws.call("txc/subscribe", {event:[
			"Long_Term_Fuel_Trim_Bank1",
			"Intake_air_pressure",
			"Accelerator_Pedal_value",
			"Fuel_consumption",
			"Torque_of_friction",
			"Maximum_indicated_engine_torque",
			"Engine_torque",
			"Calculated_LOAD_value",
			"Activation_of_Air_compressor",
			"Engine_coolant_temperature",
			"Transmission_oil_temperature",
			"Wheel_velocity_front_left-hand",
			"Wheel_velocity_front_right-hand",
			"Wheel_velocity_rear_left-hand",
			"Torque_converter_speed",

			"START",
			"STOP"]}, onSubscribed, onAbort);
	ws.call("stat/subscribe", true);
	ws.onevent("stat/stat", gotStat);
}



function onSubscribed() {
	document.body.className = "connected";
	ws.onevent("txc/Long_Term_Fuel_Trim_Bank1", gotltftb);
	ws.onevent("txc/Intake_air_pressure", gotiap);
	ws.onevent("txc/Accelerator_Pedal_value", gotapv);
	ws.onevent("txc/Fuel_consumption", gotfc);
	ws.onevent("txc/Torque_of_friction", gottof);
	ws.onevent("txc/Maximum_indicated_engine_torque", gotmiet);
	ws.onevent("txc/Engine_torque", gotet);
	ws.onevent("txc/Calculated_LOAD_value", gotclv);
	ws.onevent("txc/Activation_of_Air_compressor", gotaoac);
	ws.onevent("txc/Engine_coolant_temperature", gotect);
	ws.onevent("txc/Transmission_oil_temperature", gottot);
	ws.onevent("txc/Wheel_velocity_front_left-hand", gotwvflh);
	ws.onevent("txc/Wheel_velocity_front_right-hand", gotwvfrh);
	ws.onevent("txc/Wheel_velocity_rear_left-hand", gotwvrlh);
	ws.onevent("txc/Torque_converter_speed", gottcs);
	ws.onevent("txc/START", gotStart);
	ws.onevent("txc/STOP", gotStop);
	ws.onevent("txc",gotAny);
}

function replyok(obj) {
	document.getElementById("output").innerHTML = "OK: "+JSON.stringify(obj);
}
function replyerr(obj) {
	document.getElementById("output").innerHTML = "ERROR: "+JSON.stringify(obj);
}
function send(message) {
	var api = document.getElementById("api").value;
	var verb = document.getElementById("verb").value;
	ws.call(api+"/"+verb, {data:message}, replyok, replyerr);
}

function doConnect() {
	document.body.className = "connecting";
	ws = new afb.ws(onOpen, onAbort);
}

function doStart(fname) {
    $("#tb_load").show(500).slideDown(1000);

	ws.call('txc/start',{filename: fname});
}

function doStop() {
	ws.call('txc/stop',true);
}

$(function() {
	wdgVsp = document.getElementById("vsp");
	wdgEsp = document.getElementById("esp");
	wdgTrq = document.getElementById("trq");
	wdgStat = document.getElementById("stat");
	wdgMsg = document.getElementById("msg");

	doConnect();

	// init interval to compute message rate
	setInterval(updateMsgRate,250);
});
