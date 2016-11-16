<h1><u> Bluetooth Device Scanner</u> </h1>

<h3> An application which uses the Edge Devices Bluetooth adapter to scan for Bluetooth 4.0 Low Energy and Bluetooth 
2.4 devices. </h3>

<h4> How the Application Works</h4>
<ul>
<li> Each search for devices lasts approximately 1 minute</li>
<li> The MAC Address and device name (if available) is added to a registry of devices found within the last 1 hour</li>
<li> Every half hour the the registry of devices is cleared </li>
<li> The registry of all devices found in the last half hour, as well as all new discovered devices upon each completed search, is sent to Node-RED via a HTTP POST in JSON format. This provides some flexibility for further development</li>
<li> <b> What you need to know about Node-RED?</b>
     <ul>
        <li> The application requires Node-RED to be installed and running on the following URL: http://localhost:1880 </li>
        <li> The JSON message sent to Node-RED containing all the discovered Bluetooth devices is sent via the following URL:
        http://localhost:1880/ </li>
     
     
     </ul>
  




