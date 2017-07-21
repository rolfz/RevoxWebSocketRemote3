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
//		console.log('OM: Parsed:');
//		console.log(data);
		if((data.id)=="status"){ // means text of function to be displayed
				document.getElementById(data.id).innerHTML =  data.value;}
		else if((data.id)=="maincnt"){ // or it means the counter value to have 0 heading ie 0001
				document.getElementById(data.id).innerHTML =  pad(data.count,4);
				document.getElementById("secCnt").innerHTML =  pad(data.count,4);
				}
// 	  else if("S" in data.id || "E" in data.id){ // does not work
		else { // if it is not maincnt or status it must be the memory counters
				document.getElementById(data.id).innerHTML =  pad(data.count,4);
				// we need to replace the first char with nto display on page 2
				data.id = data.id.replace(data.id.charAt(0), "n");
				document.getElementById(data.id).innerHTML =  pad(data.count,4);
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

function sendButton(obj){
	     if(obj=="$CL" ){
			 if(confirm("Are you sure!")){
			ws.send(obj);
			console.log(obj);
			}}
		 else
		 {
		ws.send(obj);
		console.log(obj);
		 }
};

function sendInput(id, msg){
	// send data to the server
	var json = JSON.stringify({"id":id,"value":msg});
  ws.send(json);
	console.log(json);
};

function pad(num, size) {
	  if(num<0)num=num*1+10000;
    var s = "0000" + num;
    return s.substr(s.length-size);
};

function updateCounters(){
$.getJSON('/update.json', function(data){

  $('#mS1').html(pad(data.mS1,4));
  $('#mE1').html(pad(data.mE1,4));
  $('#mS2').html(pad(data.mS2,4));
  $('#mE2').html(pad(data.mE2,4));
  $('#nS1').html(pad(data.mS1,4));
	$('#nE1').html(pad(data.mE1,4));
	$('#nS2').html(pad(data.mS2,4));
	$('#nE2').html(pad(data.mE2,4));

  }).fail(function(err){
  		console.log("err getJSON mesures.json "+JSON.stringify(err));
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

  $('#B1').val(data.o1b);
  $('#B2').val(data.o2b);
  $('#B3').val(data.o3b);
  $('#B4').val(data.o4b);
  $('#B5').val(data.o5b);
  $('#F1').val(data.o1f);
  $('#F2').val(data.o2f);
  $('#F3').val(data.o3f);
  $('#F4').val(data.o4f);
  $('#F5').val(data.o5f);

  }).fail(function(err){
  		console.log("err getJSON offset.json "+JSON.stringify(err));
			});
};
//window.setInterval(updateCounters ,1000);
