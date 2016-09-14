var http = require('http');
var URL = require('url');
var fs = require('fs');
var mime = require('mime');
var exec = require('child_process').exec;
var child;
var qs = require('querystring');

// Auxiliar variables
var port = 5000;
var arduino_port = 	"/dev/ttyACM0";
var key = "";
	
//Initialization
child = exec('stty -F ' + arduino_port + ' cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts',
function  (error, stdout, stderr) {
	if (error !== null) {
		console.log('Error en la Inicialización. ¿Arduino no conectado?');
		// process.exit(1);
	} else {
		console.log('server inicializado en el puerto: ' + port);
	}
});

http.createServer(function(request, response){
	var model = {
		button: function () {
			child = exec('echo ' + key + ' > ' + arduino_port,
  			function (error, stdout, stderr) {
    			if (error !== null) {
    				console.log('exec error: ' + error);
    			}
			});
		}
	}
	var view = {
		render: function (file,r1) {
			fs.readFile('app.html', 'utf-8', function (err, app) {
				if (!err) {
					fs.readFile(file, 'utf-8', function(err, view) {
						if (!err) {
							var data = app.replace(/<%view%>/, view);
							data = data.replace(/<%r1%>/, r1);
							response.writeHead(200, {
								'Content-Type': 'text/html'
							});
							response.end(data);
						} else {
							view.error(500, "Error al renderizar la vista");
						};
					});
				} else {
					view.error(500, "Error en la vista");
				}
			});
		},

		file: function(file) {
			fs.readFile(file, function(err, data) {
				if (!err){
					response.writeHead(200, {
						'Content-Type': mime.lookup(file),
						'Content-Length': data.length
					});
					response.end(data);
				} else {
					view.error (500, file + " not found");
				};
			});
		},
		error: function(code, msg) {
			response.writeHead(code); 
		response.end(msg);
		}
	}
	var controller = {
		index: function () {
			view.render('index.html',"");
		},

		player: function () {
			view.render('player.html',"");
		},

		button: function () {
			model.button();
			view.render('index.html',"");
		},
		
		file: function () { view.file(url.pathname.slice(1)); }
	}

	var url = URL.parse(request.url, true);//
	var post_data = "";
	request.on('data', function (chunk) { post_data += chunk; });
   	request.on('end', function() {

    	post_data = qs.parse(post_data);
    	key = (post_data.button || url.query.button);
    	console.log(key);
   		var route = (post_data._method || request.method) + ' ' + url.pathname;
   		console.log('Ruta: ' + route);
    	switch (route) {
    	  	case 'GET /'		: { controller.index(); break; }		//Cambiar a controller.index()
    	  	case 'GET /player'	: { controller.player(); break; }
    	  	case 'POST /remote'	: { controller.button(); break; }
			default: {
				if (request.method == 'GET') {
					controller.file();
				} else {
					view.error(400, "Petición no contemplada ruta: " + route);
				}
			}
		}
  	});
}).listen(port);
