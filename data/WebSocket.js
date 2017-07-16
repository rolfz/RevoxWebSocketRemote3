/*
	javascript code for Revox77 wifi controller
	By Rolf Ziegler
	June 2017
*/
var ws;

//	console.log('HTML Start');
	ws = new WebSocket('ws://'+location.hostname+':81/');

	ws.onopen = function(evt){
		console.log('OO, WebSocket open');
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
		else if((data.id)=="maincnt"){ // or it means the counter value to have 0 heading ie 0001
		document.getElementById(data.id).innerHTML =  pad(data.count,4);
		document.getElementById("secCnt").innerHTML =  pad(data.count,4);
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
/*
function checkBox(obj,action){

//			console.log(obj);
//			console.log(action);
			var message ={"checkObj":obj,"checkData":action};
			ws.send(JSON.stringify(message));
};
*/
function pad(num, size) {
	  if(num<0)num=num*1+10000;
    var s = "0000" + num;
    return s.substr(s.length-size);
};
/* this function is probably not required

function updateCounter(){
$.getJSON('/update2.json', function(data){

  $('#maincnt').html(pad(data.cMa,4));

  }).fail(function(err){
  console.log("err getJSON mesures.json "+JSON.stringify(err));
});
};
*/
function updateCounters(){
$.getJSON('/update.json', function(data){

  $('#pS1').html(pad(data.c1s,4));
  $('#pE1').html(pad(data.c1e,4));
  $('#pS2').html(pad(data.c2s,4));
  $('#pE2').html(pad(data.c2e,4));
  $('#pS3').html(pad(data.c3s,4));
  $('#pE3').html(pad(data.c3e,4));
  $('#pS4').html(pad(data.c4s,4));
  $('#pE4').html(pad(data.c4e,4));
  $('#pS5').html(pad(data.c5s,4));
  $('#pE5').html(pad(data.c5e,4));

	$('#qS1').html(pad(data.c1s,4));
  $('#qE1').html(pad(data.c1e,4));
  $('#qS2').html(pad(data.c2s,4));
  $('#qE2').html(pad(data.c2e,4));
  }).fail(function(err){
  		console.log("err getJSON mesures.json "+JSON.stringify(err));
			});
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
