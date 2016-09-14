var code = {
	"up": "w",
	"left": "a",
	"down": "s",
	"right": "d",
	"pause": "p",	
}

var keys = {
	87: "w",
	65: "a",
	83: "s",
	68: "d",
	80: "p",
	32: "p",	
}

document.addEventListener('keyup', doc_keyUp, false);

function doc_keyUp(e) {
	var key = e.keyCode;
  	if (key == 87 | key == 65 | key == 83 | key == 68 | key == 80 | key == 32)
    	postCommand(keys[key]);
}

function postCommand(id) {
	var xmlhttp;
	if (window.XMLHttpRequest)
	{// code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp=new XMLHttpRequest();
	}
	else
	{// code for IE6, IE5
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}

	xmlhttp.open("POST", "/remote", true);
	xmlhttp.send("button=" + id);
}

$(document).ready(function() {
	loadStyle();
	$(".button").click(function() {
		var id=code[this.id];
		postCommand(id);
	});
	$(window).resize(function() {
		loadStyle();
	});
});

function loadStyle() {
	if ($(window).width() > $(window).height()*1.2) {
		$('#stylev').prop('disabled', true);
		$('#styleh').prop('disabled', false);	
		return("Horizontal");  	
	} else {
		$('#stylev').prop('disabled', false);
		$('#styleh').prop('disabled', true);	
		return("Vertical");
	}
}
/*
var s = 0;
function switchStyle() {
	if (s) {
		$('#stylev').prop('disabled', true);
		$('#styleh').prop('disabled', false);
		s=0;
		return("Horizontal");  	
	} else {
		$('#stylev').prop('disabled', false);
		$('#styleh').prop('disabled', true);	
		s=1;
		return("Vertical");
	}
}
*/