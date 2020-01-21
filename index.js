var mqtt = require("mqtt");
var client = mqtt.connect("mqtt://192.168.1.5");

const https = require("https");

client.on("connect", function() {
	client.subscribe("#", function(err) {
		if (!err) {
		}
		console.log("Connected to mqtt");
	});
});

var clientBuffer = {};

client.on("message", function(topic, message) {
	if (topic.indexOf("flora") === 0) {
		var topicSplit = topic.split("/");
		var mac = topicSplit[1];
		var value = topicSplit[2];
		if (!clientBuffer[mac]) {
			clientBuffer[mac] = {};
		}
		var client = clientBuffer[mac];
		client[value] = message.toString();
		console.log(client);
		if (client.hasOwnProperty("conductivity") && client.hasOwnProperty("temperature")) {
			console.log(new Date() + " - " + JSON.stringify(client));
			postToServer({ plantId: mac, data: client });
			console.log("---------");
			clientBuffer[mac] = null;
		}
	}
});

var postToServer = function(json) {
	var postData = JSON.stringify(json);

	var options = {
		hostname: "green-api.butchersworkshop.io",
		port: 443,
		path: "/log",
		method: "POST",
		headers: {
			"x-api-key": "H3Qaq3GBsv5PfjidlTorR5Xr329vc0jS5aW0ziOg",
			"Content-Type": "application/json",
			"Content-Length": postData.length
		}
	};

	var req = https.request(options, res => {
		res.on("data", d => {});
	});

	req.on("error", e => {
		console.error(e);
	});

	req.write(postData);
	req.end();
};
