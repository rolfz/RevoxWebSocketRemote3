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
		if((data.id)=="status")
		document.getElementById(data.id).innerHTML =  data.value;
		else
		document.getElementById(data.id).innerHTML =  pad(data.count,4);
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

function pad(num, size) {
    var s = "000000000" + num;
    return s.substr(s.length-size);
};

function updateCounter(){
$.getJSON('/update2.json', function(data){

  $('#maincnt').html(pad(data.cMa,4));

  }).fail(function(err){
  console.log("err getJSON mesures.json "+JSON.stringify(err));
});
};

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

  }).fail(function(err){
  console.log("err getJSON mesures.json "+JSON.stringify(err));
});
};

//window.setInterval(updateCounters ,1000);
