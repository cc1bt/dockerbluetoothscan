<h1><u> Bluetooth Device Scanner</u> </h1>

<h3> An application which uses the Edge Devices Bluetooth adapter to scan for Bluetooth 4.0 Low Energy and Bluetooth 
2.4 devices. </h3>
<h4> Requirements </h4>
<ul>
<li> Ubuntu 16.04 LTS or lower </li>
<li> Bluetooth Adapter </li>
<li> Node-RED development environment running on <i>http://localhost:1880</i></li>
</ul>

<h4> How the Application Works</h4>
<ul>
<li> Each search for devices lasts approximately 1 minute</li>
<li> The MAC Address and name (if available) of the Bluetooth device found is added to a registry of devices found within the last 1 hour</li>
<li> Every half hour the the registry of devices is cleared </li>
<li> The registry of all devices found in the last half hour, as well as all new discovered devices upon each completed search, is sent to Node-RED via a HTTP POST in JSON format. This provides some flexibility for further development</li>
<li> <b> What you need to know about Node-RED?</b>
     <ul>
        <li> The application requires Node-RED to be installed and running on the following URL: <i>http://localhost:1880</i> </li>
        <li> The JSON message sent to Node-RED containing all the discovered Bluetooth devices is sent via the following URL:
        <i>http://localhost:1880/all_discovered</i> </li>
        <li> The JSON message sent to Node-RED containing new discovered Bluetooth devices is sent via the following URL:
        <i>http://localhost:1880/new_discovered </i></li>  
     </ul>
</li>
</ul>

<h4> Other Information </h4>
<ul>
<li> Written in C </li>
<li> Uses BlueZ Linux Bluetooth library </li>
<li> Open-source </li>
</ul>

  




