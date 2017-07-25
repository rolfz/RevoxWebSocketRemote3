/*
	javascript code for Revox77 wifi controller
	By Rolf Ziegler
	June 2017
*/
var ws;

//	console.log('HTML Start');
	ws = new WebSocket('ws://'+location.hostname+':81/');

	ws.onopen = function(evt){
//		console.log('OO, WebSocket open');
		ws.send('Connect ' + new Date());
		updateCounters();
  	updateOffsets();
		initDisplay();
	};

	ws.onclose = function(evt){
		console.log('OC= WebSocket connection closed');
	};

	ws.onerror = function (err) {
		console.log('OE= WebSocket Error ', err);
	};

	ws.onmessage = function (evt) {
	//	console.log('OM: Received from Server: ', evt);
		var data = JSON.parse(evt.data);
	//	console.log('OM: Parsed:');
	//	console.log(data);
		if((data.id)=="status"){ // means text of function to be displayed
				document.getElementById(data.id).innerHTML =  data.value;}
		else if((data.id)=="mainCnt"){ // or it means the counter value to have 0 heading ie 0001
				document.getElementById(data.id).innerHTML =  pad(data.count,4);
				document.getElementById("newCnt").value =  pad(data.count,4); // input line needs .value
				}
// 	  else if("S" in data.id || "E" in data.id){ // does not work
		else { // if it is not maincnt or status it must be the memory counters
				document.getElementById(data.id).innerHTML =  pad(data.count,4);
				document.getElementById(data.id).value =  pad(data.count,4);
				// we need to replace the first char with nto display on page 2
				data.id = data.id.replace(data.id.charAt(0), "m");
				document.getElementById(data.id).innerHTML=  pad(data.count,4);
				document.getElementById(data.id).value=  pad(data.count,4);
				}
	};

 function initDisplay(){
  // When ready...
      window.addEventListener("load",function() {
      	// Set a timeout...
      	setTimeout(function(){
      		// Hide the address bar!
      		window.scrollTo(0, 1);
      	}, 0);
  });
};

// this function sends 1 value as a message (for buttons)
function sendButton(obj){
	     if(obj=="$CL" ){
			 if(!confirm("Are you sure!")){
				 return;
					}
			 }
		ws.send(obj);
		console.log(obj);

};

// this function sends 2 values in a JSON string
function sendInput(id, msg){
	// check if value is 	legal numeric
		if(isNaN(msg)){
			alert("! not a numeric value");
			//document.getElementById(data.id).value=0;
			return;
			}
		//msg=msg.substr(msg.length-4);
		if(id=="newCnt"){
		 			if(!confirm("Are you sure!")){
						return;
					}
		}
	// send data to the server
	var json = JSON.stringify({"id":id,"value":msg});
  ws.send(json);
	console.log(json);
};

function pad(num, size) {
	  if(num<0)num=num*1+10000;
    var s = "0000" + String(num);
    return s.substr(s.length-size);
};

function updateCounters(){
$.getJSON('/update.json', function(data){

  $('#mS1').html(pad(data.mS1,4));
  $('#mE1').html(pad(data.mE1,4));
  $('#mS2').html(pad(data.mS2,4));
  $('#mE2').html(pad(data.mE2,4));
	$('#mS3').html(pad(data.mS3,4));
  $('#mE3').html(pad(data.mE3,4));
	$('#nS1').val(pad(data.mS1,4));
	$('#nE1').val(pad(data.mE1,4));
	$('#nS2').val(pad(data.mS2,4));
	$('#nE2').val(pad(data.mE2,4));
	$('#nS3').val(pad(data.mS3,4));
  $('#nE3').val(pad(data.mE3,4));

  }).fail(function(err){
  		console.log("err getJSON update.json "+JSON.stringify(err));
			});
};

  // on toggle method
  // to check status of checkbox
 function onToggle() {
    // check if checkbox is checked
		var msg;
    if (document.querySelector('#zero').checked) {
      // if checked
    	msg="%1";
    } else {
      // if unchecked
			msg="%0";
    }
//	var json = JSON.stringify({"id":zero,"value":msg});
		console.log(msg);
		ws.send(msg);
  };

function updateOffsets(){
$.getJSON('/offset.json', function(data){

  $('#oB1').val(data.o1b);
  $('#oB2').val(data.o2b);
  $('#oB3').val(data.o3b);
  $('#oB4').val(data.o4b);
  $('#oB5').val(data.o5b);
	$('#oB6').val(data.o6b);
  $('#oF1').val(data.o1f);
  $('#oF2').val(data.o2f);
  $('#oF3').val(data.o3f);
  $('#oF4').val(data.o4f);
  $('#oF5').val(data.o5f);
	$('#oF6').val(data.o6f);

  }).fail(function(err){
  		console.log("err getJSON offset.json "+JSON.stringify(err));
			});
};
//window.setInterval(updateCounters ,1000);
