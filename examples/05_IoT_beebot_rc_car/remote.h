
const char *REMOTE_HTML_CONTENT = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>ESP8266 Control Car via Web</title>
<meta name="viewport" content="width=device-width, initial-scale=0.7, maximum-scale=1, user-scalable=no">
<style type="text/css">
body { text-align: center; font-size: 24px;}
button { text-align: center; font-size: 24px;}
.arrow {
  	border: solid black;
  	border-width: 0 3px 3px 0;
  	display: inline-block;
  	padding: 40px;
}

.right {
	position: relative;
	background-size: contain;   
    	right: 15px;
    	top: 150px;
  	-webkit-transform: rotate(-45deg);
}

.left {
	position: relative;
	background-size: contain;
    	right:120px;
    	top: 60px;
  	-webkit-transform: rotate(135deg);
}

.up {
     position: relative;
     background-size: contain; 
     left: 45px;
     top: 33px;
     -webkit-transform: rotate(-135deg);
}

.down {
   	position: relative;
   	background-size: contain; 
    	right:43px;
    	top:264px;
  	-webkit-transform: rotate(45deg);
}

.stop {
	position: relative;
   	background-size: contain; 
	border: solid black;
  	border-width: 3px 3px 3px 3px;
    	left:130px;
   	top:145px;
  	height: 75px;
  	width: 75px;
  	background-color: red;
  	border-radius: 50%;
  	display: inline-block;
}

#container {
    	margin-right: auto;
    	margin-left: auto;
    	width: 400px; 
    	height: 400px;
    	position: relative;
    	margin-bottom: 10px;
}

div[class^='button'] { position: absolute; }

.button_push { width:80px; height:40px;}

.button_servo_c { width:122px; height:32px;}

.button_servo_a { width:160px; height:35px;}

.button_servo { width:82px; height:23px;}

.button_push {
  background: none no-repeat; 
  background-size: contain;
    left:-62px;
    top: 200px;
  transform: translate(-50%, -50%);
}

.button_servo_c {
  background: none no-repeat; 
  background-size: contain;
    right:-214px;
    top: 230px;
  transform: translate(-50%, -50%);
}

.button_servo_a {
  background: none no-repeat; 
  background-size: contain;
    right:-270px;
    top: 170px;
  transform: translate(-50%, -50%);
}

.button_servo {
  background: none no-repeat; 
  background-size: contain;
    right:-150px;
    top: 200px;
  transform: translate(-50%, -50%);
}

</style>
<script>
var CMD_STOP     = 0;
var CMD_FORWARD  = 1;
var CMD_BACKWARD = 2;
var CMD_LEFT     = 4;
var CMD_RIGHT    = 8;
var CMD_PUSH     = 5;
var CMD_SRVCLCK  = 6;
var CMD_SRVACLCK = 9;
var img_name_lookup = {
  [CMD_STOP]:     "stop",
  [CMD_FORWARD]:  "up",
  [CMD_BACKWARD]: "down",
  [CMD_LEFT]:     "left",
  [CMD_RIGHT]:    "right"
}
var ws = null;

function init() 
{
  
  var container = document.querySelector("#container");
    container.addEventListener("touchstart", mouse_down);
    container.addEventListener("touchend", mouse_up);
    container.addEventListener("touchcancel", mouse_up);
    container.addEventListener("mousedown", mouse_down);
    container.addEventListener("mouseup", mouse_up);
    container.addEventListener("mouseout", mouse_up);    
}
function ws_onmessage(e_msg)
{
    e_msg = e_msg || window.event; // MessageEvent
 
    //alert("msg : " + e_msg.data);
}
function ws_onopen()
{
  document.getElementById("ws_state").innerHTML = "OPEN";
  document.getElementById("wc_conn").innerHTML = "Disconnect";
}
function ws_onclose()
{
  document.getElementById("ws_state").innerHTML = "CLOSED";
  document.getElementById("wc_conn").innerHTML = "Connect";
  console.log("socket was closed");
  ws.onopen = null;
  ws.onclose = null;
  ws.onmessage = null;
  ws = null;
}
function wc_onclick()
{
  if(ws == null)
  {
    ws = new WebSocket("ws://" + window.location.host + ":81");
    document.getElementById("ws_state").innerHTML = "CONNECTING";
    
    ws.onopen = ws_onopen;
    ws.onclose = ws_onclose;
    ws.onmessage = ws_onmessage; 
  }
  else
    ws.close();
}
function mouse_down(event) 
{
  if (event.target !== event.currentTarget) 
  {
    var id = event.target.id;
    send_command(id);
    event.target.style.backgroundColor = "cyan";
    }
    event.stopPropagation();    
    event.preventDefault();    
}

function mouse_up(event) 
{
  if (event.target !== event.currentTarget) 
  {
    var id = event.target.id;
    send_command(CMD_STOP);
    event.target.style.backgroundColor = "white";
    }
    event.stopPropagation();   
    event.preventDefault();    
}

function wc_onclickpush() 
{   
  send_command(CMD_PUSH);   
}
function wc_onclickservclck() 
{   
  send_command(CMD_SRVCLCK);   
}
function wc_onclickservaclck() 
{   
  send_command(CMD_SRVACLCK);   
}

function send_command(cmd) 
{   
  if(ws != null)
    if(ws.readyState == 1)
      ws.send(cmd + "\r\n");   
}

window.onload = init;
</script>
</head>
<body>
<h2>ESP8266 - RC Car via Web</h2>
<div id="container">
    <div id="0" class="stop"></div>
    <div id="1" class="arrow up"></div>
    <div id="2" class="arrow down"></div>
    <div id="8" class="arrow right"></div>
    <div id="4" class="arrow left"></div>
</div>
<p>
WebSocket : <span id="ws_state" style="color:blue">closed</span>
<button id="wc_conn" type="button" onclick="wc_onclick();">Connect</button><br>
</p>
<button id="wc_push" type="button" onclick="wc_onclickpush();">Push</button>
<h style="font-size:17px; color:blue">SERVO</h>
<button id="wc_servclck" type="button" onclick="wc_onclickservclck();">Clockwise</button>
<button id="wc_servaclck" type="button" onclick="wc_onclickservaclck();">AntiClockwise</button>
<br>
<br>
<div class="sponsor">Sponsored by <a href="https://paulsayan.wixsite.com/buildybee">Buidybee</a></div>
</body>
</html>
)=====";
