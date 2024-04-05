const char *HTML_LIGHT = R"=====(
  <!DOCTYPE html>
<html>
  <head>
    <title>wifi pixel</title>
    <meta name="viewport" content="width=device-width, height=device-height, initial-scale=1.0, user-scalable=0, minimum-scale=1.0, maximum-scale=1.0" />
    <style>
      body { background-color: #F08262; font-family: Arial; Color: #fff; }
      h1 {text-align: center;}
      p {text-align: center;}
      form {text-align: center;}
    </style>
    <script>
      var color = "c";
      var ws = null;
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
function wc_color() 
{   
  send_command(document.getElementById("colorPicker").value);   
}
function send_command(cmd) 
{   
  if(ws != null)
    if(ws.readyState == 1)
      ws.send(cmd + "\r\n");   
}
    </script>
  </head>
  <body>
    <h1>Buildybee</h1>
    <p>pick the color!</p>
    <p> <input id = "colorPicker" type="color" name="c" value="%02d" onchange="wc_color();" /> 
        &nbsp;<span onclick="wc_color();" style="font-size:16pt;"> </span>
    </p>
    <p>
	      WebSocket : <span id="ws_state" style="color:blue">closed</span>
	      <button id="wc_conn" style="text-align: center; font-size: 24px;" onclick="wc_onclick();">Connect</button><br>
  </p>
  </body>
</html>
)=====";