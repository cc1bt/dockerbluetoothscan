#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/socket.h>
#include "bluetooth.h"
#include  "hci.h"
#include "hci_lib.h"
#include "vector.h"
#include "http_requests.h"
#include "bluetooth_scanner.h"

//variables
vector processedDevices; //main device list

int main(){
	//initialise the vectors
	VECTOR_INIT(processedDevices);

	time_t timeProcessedDevicesCleared = time (0); //time of when the processedDevices is cleared

	int repeat = 0;
	while(repeat == 0){
		VECTOR_INIT(devicesFound);

		//scan for Bluetooth 4.0 devices
		devicesFound = scanBLE(devicesFound);
		sleep(8); //wait 5 seconds
		//scan for Bluetooth 2.4 devices
		devicesFound = scanBluetooth(devicesFound);

		/**
		* Get the current time then calculate the difference of time between now and the time in the timeProcessedDevicesCleared
		* variable which is the time when the processedDevices vector was cleared.
		* The time difference is returned from difftime() function in seconds. If the difference in time is longer than 
		* half hour loop through each element in processedElements checking if each element was processed #
		* more than 15 mins ago, remove from vector.
		*/
		time_t timeNow = time (0);
		int timeToClean = difftime(timeNow, timeProcessedDevicesCleared); 

		//check time dif
		if(timeToClean >= 1800){
			int totalProcessedDevices = VECTOR_TOTAL(processedDevices);
			for(int i = 0; i < totalProcessedDevices; i ++){
				//get elements processed time & create time_t variable to check the time difference
				int timeProcessedInt = VECTOR_GET_TIME(processedDevices, int, i);
				time_t timeProcessed = timeProcessedInt;
				int timeDifference =  difftime(timeNow, timeProcessed);
				if(timeDifference >= 600){ //if time difference is > than 15 mins delete element
					VECTOR_DELETE(processedDevices, i);
				}
			}
			//update time list was processed with the current time 
			timeProcessedDevicesCleared = timeNow;
		}

		//format the timeNow timestamp into a string to add to json message
		char timeNowStr[100];
		strftime (timeNowStr, 100, "%Y-%m-%d %H:%M:%S", localtime (&timeNow));

		//create json message string to send to node red.
		char jsonmsg[10000]; //to send to node-red
		char jsonmsgNames[5000]; //store the device names, to then append to jsonmsg

		strcat(jsonmsgNames, "=, \"Device_Names\":[");
		strcat(jsonmsg, "{\"Date\":\"");
		strcat(jsonmsg, timeNowStr);
		strcat(jsonmsg, "\", \"MAC_Addresses\":[");

		/**
		* Loop through each of the found devices & check if it is the processedDevices list.
		* If it is, get the processed time of that devices from processedDevices list and the time the device was now found
		* in devicesFound. If the difference between times is > 15 mins, the device should be processed again. To do this add
		* the devices details to the json message & update the processed time in processedDevices list.
		* If device is not found in processedDevices list, it must be processed. The devices details are added to the 
		* json message & a new entry is added to processedDevices list.
		*/
		int somethingToSend = 1; //flag which checks if there is something to send to Node-RED or not
		int totalFound = VECTOR_TOTAL(devicesFound);
		for(int i = 0; i < totalFound; i++){
			char *newDeviceMacAddress = VECTOR_GET_MACADDRESS(devicesFound, char*, i);
			int timeOfTempInt = VECTOR_GET_TIME(devicesFound, int , i);
			time_t timeOfTemp = timeOfTempInt;
			if(vector_contains(&processedDevices,  newDeviceMacAddress) == 0){//this device has been previously processed
				//get the processed device index
				int processedDeviceIndex = vector_get_index(&processedDevices, newDeviceMacAddress);
				//use the index to get the time it was previously processed
				int timeOfProcessedInt = VECTOR_GET_TIME(processedDevices, int , processedDeviceIndex);//time it was processed
				time_t timeOfProcessed = timeOfProcessedInt;
				int timeDifference = difftime(timeOfTemp, timeOfProcessed);

				//code for debugging purposes
//				char time1[100];
//				time_t timee1 = timeOfProcessed;
//				strftime (time1, 100, "%Y-%m-%d %H:%M:%S", localtime (&timee1));
//				char time2[100];
//				time_t timee2 = timeOfTemp;
//				strftime (time2, 100, "%Y-%m-%d %H:%M:%S", localtime (&timee2));
//
//				printf("--- time of processed...%s\n", time1);
//				printf("--- time of temp...%s\n", time2);
//				printf("--- time difference in secs.. %d\n", timeDifference);

				if(timeDifference  > 600){ //check time difference is > 15 mins
					char *deviceName = VECTOR_GET_NAME(devicesFound, char*, i);
					strcat(jsonmsgNames, "\"");
					strcat(jsonmsg, "\"");
					strcat(jsonmsgNames, deviceName);
					strcat(jsonmsg, newDeviceMacAddress);
					strcat(jsonmsgNames, "\"");
					strcat(jsonmsg, "\"");
					strcat(jsonmsg, ",");
					strcat(jsonmsgNames, ",");

					VECTOR_SET_TIME(processedDevices, processedDeviceIndex, timeOfTemp);
					//set flag as true
					somethingToSend = 0;

				}
			}else{//device has not been previously processed
				char *deviceName = VECTOR_GET_NAME(devicesFound, char*, i);
				//add to json msg
				strcat(jsonmsgNames, "\"");
				strcat(jsonmsg, "\"");
				strcat(jsonmsgNames, deviceName);
				strcat(jsonmsg, newDeviceMacAddress);
				strcat(jsonmsgNames, "\"");
				strcat(jsonmsg, "\"");
				strcat(jsonmsg, ",");
				strcat(jsonmsgNames, ",");

				char *processed = "true";

				//add to processedDevices list
				VECTOR_ADD(processedDevices, newDeviceMacAddress, deviceName , processed , (int *)timeOfTemp);
				//set flag as true
				somethingToSend = 0;
			}
		}

		//parameters required to send to Node-RED
		char *hostname = "10.255.21.45";
		char *page = "/bluetoothscanner";
		int port = 1880;

		//check if the flag is true (there is something to send)
		if(somethingToSend == 0){
			//do some formatting to the json string & end the message
			strcat(jsonmsgNames, "~}\0");
			char *ptrchar = strchr(jsonmsgNames, '~');
			int indexToChar = (int)(ptrchar - jsonmsgNames);
			jsonmsgNames[indexToChar] = ']';
			jsonmsgNames[indexToChar-1] = ' ';


			strcat(jsonmsg, jsonmsgNames);
			ptrchar = strchr(jsonmsg, '=');
			indexToChar = (int)(ptrchar - jsonmsg);
			jsonmsg[indexToChar] = ']';
			jsonmsg[indexToChar-1] = ' ';

			//printf("\n--- JSON --- %s\n", jsonmsg);

			//send to Node-RED
			createHTTP(hostname, page, jsonmsg, port);

			//set flag back to false
			somethingToSend = 1; 
		}
		//clear json message chars
		memset(&jsonmsg[0], 0, sizeof(jsonmsg));
		memset(&jsonmsgNames[0], 0, sizeof(jsonmsgNames));

		//print processed devices (debugging purposes)
		char jsonAddresses[5000];
		char jsonNames[5000];
		char jsonDates[5000];

		strcat(jsonAddresses, "{\"Addresses\":[");
		strcat(jsonNames, "\"Names\":[");
		strcat(jsonDates, "\"Dates\":[");
		printf("\n-------------------------------------------------\n");
		int totalproc = VECTOR_TOTAL(processedDevices);
		for(int i = 0; i < totalproc; i ++){
			char *macAddress = VECTOR_GET_MACADDRESS(processedDevices, char*, i);
			char *deviceName = VECTOR_GET_NAME(processedDevices, char*, i);
			char *processed = VECTOR_GET_PROCESSED(processedDevices, char*, i);
			int time = VECTOR_GET_TIME(processedDevices, int, i);
			//convert timestamp to readable string
			char buff[100];
			time_t now = time;
			strftime (buff, 100, "%Y-%m-%d at %H:%M:%S", localtime (&now));

			printf("\n%s ", macAddress);
			printf("  %s ", deviceName);
			printf("  %s ", processed);
			printf("  %s ", buff);

			//format json arrays to send to Node-RED a full list of processed devices
			strcat(jsonAddresses, "\"");
			strcat(jsonNames, "\"");
			strcat(jsonDates, "\"");

			strcat(jsonAddresses, macAddress);
			strcat(jsonNames, deviceName);
			strcat(jsonDates, buff);

			strcat(jsonAddresses, "\"");
			strcat(jsonNames, "\"");
			strcat(jsonDates, "\"");

			if(i != (totalproc-1)){
				strcat(jsonAddresses, ",");
				strcat(jsonNames, ",");
				strcat(jsonDates, ",");
			}
		}
		//end json strings
		strcat(jsonAddresses, "],");
		strcat(jsonNames, "],");
		strcat(jsonDates, "]}");
		strcat(jsonAddresses, jsonNames);
		strcat(jsonAddresses, jsonDates);
		//clear memory
		memset(&jsonDates[0], 0, sizeof(jsonDates));
		memset(&jsonNames[0], 0, sizeof(jsonNames));

		//send to Node-RED
		page = "/processedDevices";
		createHTTP(hostname, page, jsonAddresses, port);

		memset(&jsonAddresses[0], 0, sizeof(jsonAddresses));

		printf("\n-------------------------------------------------\n");

		VECTOR_FREE(devicesFound);
	}
    	VECTOR_FREE(processedDevices);
	return 0;
}


