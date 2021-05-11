var testDescription = {
	title: "Zebra7500 unit test",
    id: "",
    id_in: "",
    id_out: "",
	steps: [
        {
            name: "moduleZebra7500_Step1_Start",
            mode: "", // Can be: flow (default), loop, event
            event: "" // event name that will call this function
        }, 
        {
            name: "moduleZebra7500_Step1_Loop",
            mode: "loop", // Can be: flow (default), loop, event
            event: "" // event name that will call this function
        }
	]
}

moduleZebra7500_Step1_Start = function()
{
    nfc_reader('START');
}

moduleZebra7500_Step1_Loop = function()
{ 
    //     printLog("info", "Zebra step 1");
    delay1s(1);
}


exports.testDescription = testDescription;
