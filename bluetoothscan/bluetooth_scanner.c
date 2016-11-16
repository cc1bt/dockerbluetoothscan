#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "bluetooth_scanner.h"
#include "vector.h"

//Structure for hci requast to scan Bluetooth 4.0 Low Energy devices
struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam){
	struct hci_request hciRequest;
	memset(&hciRequest, 0, sizeof(hciRequest));
	hciRequest.ogf = OGF_LE_CTL;
	hciRequest.ocf = ocf;
	hciRequest.cparam = cparam;
	hciRequest.clen = clen;
	hciRequest.rparam = status;
	hciRequest.rlen = 1;
	return hciRequest;
}

/**
 * Function which scans for Bluetooth 4.0 low energy devices & places the devices MAC address in
 * the passed vector parameter, which it returns at the end.
 */
vector scanBLE(vector blVector){
	int status;

	// Get HCI device.
	int thisDeviceID = hci_get_route(NULL);
	//open device for scanning - has to be a number > 0  - if its < 0 exit program
	const int thisDevicesBluetooth = hci_open_dev(thisDeviceID);
	if (thisDevicesBluetooth < 0 ) {
		perror("inside scanble");
		perror("Failed to open HCI device.");
		exit(1);
	}

	//Set BLE scan parameters - if it fails exit program
	le_set_scan_parameters_cp scan_params_cp;
	//set scan parameters to 0
	memset(&scan_params_cp, 0, sizeof(scan_params_cp));
	scan_params_cp.type 			= 0x00;
	scan_params_cp.interval 		= htobs(0x0010);
	scan_params_cp.window 			= htobs(0x0010);
	scan_params_cp.own_bdaddr_type 	= 0x00; // Public Device Address (default).
	scan_params_cp.filter 			= 0x00; // Accept all.

	//Create a hci request for the scan params to initialise/set them
	struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);
	if (hci_send_req(thisDevicesBluetooth, &scan_params_rq, 1000) < 0 ) {
		hci_close_dev(thisDevicesBluetooth);
		perror("Failed to set scan parameters data.");
        exit(1);
	}

	//Set BLE events report mask - if it fails exit program
	le_set_event_mask_cp event_mask_cp;
	//set scan parameters to 0
	memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));
	for (int i = 0 ; i < 8 ; i++ ) event_mask_cp.mask[i] = 0xFF;

	//Create a hci request to set the events report mask
	struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
	if (hci_send_req(thisDevicesBluetooth, &set_mask_rq, 1000) < 0 ) {
		hci_close_dev(thisDevicesBluetooth);
		perror("Failed to set event mask...");
        exit(1);
	}

	// Enable scanning - if it fails exit program
	le_set_scan_enable_cp bleScan;
	memset(&bleScan, 0, sizeof(bleScan));
	bleScan.enable 		= 0x01;	// Enable flag.
	bleScan.filter_dup 	= 0x00; // Filtering disabled.

	//Create a hci request enable scanning for BLE devices
	struct hci_request enableScanningRequest = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &bleScan);
	if(hci_send_req(thisDevicesBluetooth, &enableScanningRequest, 1000) < 0 ) {
		hci_close_dev(thisDevicesBluetooth);
		perror("Failed to enable scan.");
        exit(1);
	}

	//Get Results
	struct hci_filter nf;
	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(thisDevicesBluetooth, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0 ) {
		hci_close_dev(thisDevicesBluetooth);
		perror("Could not set socket options\n");
        exit(1);
	}

	printf("\nScanning LE Devices....\n");

	uint8_t buf[HCI_MAX_EVENT_SIZE];
	evt_le_meta_event * meta_event;
	le_advertising_info * deviceFoundInfo;
	int len;

	//set the time it should scan devices for - 20 seconds
	time_t timeToScanFor = 20;
	//start timer
	time_t start = time(NULL);

	time_t timeNow = time(0);
	int *timeInt = timeNow;
	while (time(NULL) - start < timeToScanFor) { //check if time elapsed is < time it should scan for
		len = read(thisDevicesBluetooth, buf, sizeof(buf));
		char macAddress[18];
		char deviceName[248];
		char *processed = "false";
		if ( len >= HCI_EVENT_HDR_SIZE ) {
			meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
			if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
				void * offset = meta_event->data + 1;
				deviceFoundInfo = (le_advertising_info *)offset;
				//get device MAC address
				ba2str(&(deviceFoundInfo->bdaddr), macAddress);
				//set the deviceName list indexes all to 0
				memset(deviceName, 0, sizeof(deviceName));

				if (hci_read_remote_name(thisDevicesBluetooth, &(deviceFoundInfo)->bdaddr, sizeof(deviceName), deviceName, 0) < 0)
					strcpy(deviceName, "Device Name Unknown");

				char *addressCpy = malloc(18 * sizeof(char));
				strcpy(addressCpy, macAddress);

		        char *deviceNameStr = malloc(248 * sizeof(char));
		       	strcpy(deviceNameStr, deviceName);
				if(VECTOR_TOTAL(blVector) != 0){
					if(vector_contains(&blVector, addressCpy) == 1){
						printf("%s %s\n", macAddress, deviceName);
						VECTOR_ADD(blVector, addressCpy, deviceNameStr, processed, timeInt);
					}
				}else{
					printf("%s %s\n", macAddress, deviceName);
					VECTOR_ADD(blVector, addressCpy, deviceNameStr,processed, timeInt);
				}
			}
		}


	}

	// Disable scanning - if it fails to disable exit the program
	memset(&bleScan, 0, sizeof(bleScan));
	bleScan.enable = 0x00;	// Disable flag.
	struct hci_request disableScanRequest = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &bleScan);
	if (hci_send_req(thisDevicesBluetooth, &disableScanRequest, 1000) < 0 ) {
		hci_close_dev(thisDevicesBluetooth);
		perror("Failed to disable scan.");
        exit(1);
	}

	//close device
	hci_close_dev(thisDevicesBluetooth);

	/*status = 0;
	thisDeviceID = 0;
    memset(&scan_params_cp, 0, sizeof(scan_params_cp));
    memset(&scan_params_rq, 0, sizeof(scan_params_rq));
    memset(&enableScanningRequest, 0, sizeof(enableScanningRequest));
	memset(&event_mask_cp, 0, sizeof(event_mask_cp));
	memset(&set_mask_rq, 0, sizeof(set_mask_rq));
	memset(&bleScan, 0, sizeof(bleScan));
	memset(&nf, 0, sizeof(nf));
	memset(&buf[0], 0,sizeof(buf));
	memset(&meta_event, 0, sizeof(meta_event));
	memset(&timeToScanFor, 0, sizeof(timeToScanFor));
	memset(&start, 0, sizeof(start));
	len = 0; */

	return blVector;
}

/*
 * Function which scans for Bluetooth 2.4 devices & places the devices MAC address in
 * the passed vector parameter, which it returns at the end.
 *
 */
vector scanBluetooth(vector blVector){
    printf("\nScanning Bluetooth Devices...\n");
	 inquiry_info *devices = NULL;
	    int maxNumberOf_foundDevices, numberOf_foundDevices;
	    int thisDeviceID, thisDevicesBluetooth, len, flags;

	    thisDeviceID = hci_get_route(NULL);
	    thisDevicesBluetooth = hci_open_dev( thisDeviceID );
	    if (thisDeviceID < 0 || thisDevicesBluetooth < 0) {
	        perror("Error setting up Bluetooth for scanning...");
	        exit(1);
	    }

	    len = 8; //??
	    maxNumberOf_foundDevices = 255;
	    flags = IREQ_CACHE_FLUSH;
	    devices = (inquiry_info*)malloc(maxNumberOf_foundDevices * sizeof(inquiry_info));

	    numberOf_foundDevices = hci_inquiry(thisDeviceID, len, maxNumberOf_foundDevices, NULL, &devices, flags);
	    if( numberOf_foundDevices < 0 ){
	    	perror("Error with hci_inquiry function");
	    }
		time_t timeNow = time(0);
		int *timeInt = timeNow;
	    for (int i = 0; i < numberOf_foundDevices; i++) {
	    	char deviceMacAddress[18];
	    	char deviceName[248];
			char *processed = "false";

	        ba2str(&(devices+i)->bdaddr, deviceMacAddress);
	        memset(deviceName, 0, sizeof(deviceName));
	        if (hci_read_remote_name(thisDevicesBluetooth, &(devices+i)->bdaddr, sizeof(deviceName), deviceName, 0) < 0)
	        	strcpy(deviceName, "Device Name Unkown");


	        char *addressCpy = malloc(18 * sizeof(char));
	        strcpy(addressCpy, deviceMacAddress);

	        char *deviceNameStr = malloc(248 * sizeof(char));
	       	strcpy(deviceNameStr, deviceName);

			if(VECTOR_TOTAL(blVector) != 0){
				if(vector_contains(&blVector, addressCpy) == 1){
			        printf("%s %s\n", deviceMacAddress, deviceName);
					VECTOR_ADD(blVector, addressCpy, deviceNameStr, processed, timeInt);
				}
			}else{
		        printf("%s %s\n", deviceMacAddress, deviceName);
				VECTOR_ADD(blVector, addressCpy, deviceNameStr, processed, timeInt);
			}
	    }

	    free(devices);
	    close(thisDevicesBluetooth);
	    return blVector;
}
