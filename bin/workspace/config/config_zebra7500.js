
// ===============================================================================================
// DEVICES CONFIGURATION
// ===============================================================================================

/**
	conn_channel parameters value:
	

	conn_channel identifier examples:
	   - Explicit: "COM16"
	   - Using VID/PID couple: "1234:5678"
	   - Using the interface name:  "Silicon Labs CP210x USB to UART Bridge"
	   - Using the COM port serial number (eg, FTDI, Cypress chips): "AB98986"

	Peripheral discovery policy:
	  1. Scan for computer peripheral
	  2. Try to associate using the defined conn_channel
	  3. Try to associate using the defined peripheral id

*/

function setup()
{
	return {
	admin_pass: "caravane",
	devices: [
    {
        name: "nfc_reader",
        type: "Zebra7500",
        conn_channel: "10.0.0.20",
        conn_settings: "",
        id: "", 
        options: ""
    }
    ]};
}

