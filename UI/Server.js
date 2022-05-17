const express = require('express');
const app = express();
const http = require('http');
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server);

const mqtt = require('mqtt');
var nsUrl = require("url");
var nsPath = require("path");
var nsFs = require("fs");

const host = 'localhost'
const port = '1883'
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`



const connectUrl = `mqtt://${host}:${port}`

const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: 'my_user',
  password: 'bendevictor',
  reconnectPeriod: 1000,
})


const topic = 'temp'
client.on('connect', () => {
  console.log('Connected')
  client.subscribe(["temp"], () => {
    console.log(`Subscribe to topic temp`)
  })
  client.subscribe(["co2"], () => {
    console.log(`Subscribe to topic message`)
  })
  client.subscribe(["humidity"], () => {
    console.log(`Subscribe to topic message`)
  })
  client.subscribe(["windowActuator"], () => {
    console.log(`Subscribe to topic windowActuator`)
  })
  client.subscribe(["dehumidifierActuator"], () => {
    console.log(`Subscribe to topic message`)
  }),
  client.subscribe(["tempActuator"], () => {
    console.log(`Subscribe to topic message`)
  })
  client.subscribe(["light"], () => {
    console.log(`Subscribe to topic light`)
  })
  client.subscribe(["moisture"], () => {
    console.log(`Subscribe to topic moisture`)
  })
  client.subscribe(["manual"], () => {
    console.log(`Subscribe to topic moisture`)
  })
})

client.on('message', (topic, payload) => {
    console.log('Received Message:', topic, payload.toString())
    if (topic == "temp"){
      io.emit("temp", payload.toString())
    }
    if (topic == "co2"){
      io.emit("co2", payload.toString())
    }
    if (topic == "humidity"){
      io.emit("humidity", payload.toString())
    }
    if (topic == "windowActuator"){
      io.emit("windowActuator", payload.toString())
    }
    if (topic == "dehumidifierActuator"){
      io.emit("dehumidifierActuator", payload.toString())
    }
    if (topic == "tempActuator"){
      io.emit("tempActuator", payload.toString())
    }
    if (topic == "light"){
      io.emit("light", payload.toString())
    }
    if (topic == "moisture"){
      io.emit("moisture", payload.toString())
    }
    if (topic == "manual"){
      io.emit("manual", payload.toString())
    }
})
// reads a file contents and sends, but if any error occur,
// sends a 500 HTTP Status Code (Internal Server Error)




app.get('/', (req, res) => {
  res.sendFile(__dirname + '/website.html');
});

app.get('/service', (req, res) => {
  res.sendFile(__dirname + '/website.html');
});

app.post('/publish', (req, res) => {
  var body = ''
  req.on('data', function(data) {
    body += data
    console.log('Partial body: ' + body)
  })
  req.on('end', function() {
    console.log('Body: ' + body)
    var parsed = JSON.parse(body)
    console.log("sending" + JSON.stringify(parsed))
    client.publish(parsed.topic, parsed.message);
  })
});
app.get('/alert', (req, res) => {
  res.sendFile(__dirname + '/website.html');
});


io.on('connection', (socket) => {
  io.emit("temp", 21)
  socket.on("fanspeed", msg => {
    console.log("changing fan to:" + msg)
    client.publish("fan", msg)
  });
  socket.on("fanspeed", msg => {
    console.log("changing fan to:" + msg)
    client.publish("fan", msg)
  });
  socket.on("publish", msg => {
    var parsed = JSON.parse(msg)
    console.log("sending" + JSON.stringify(parsed))
    client.publish(parsed.topic, parsed.message);
  });
});


server.listen(8080, () => {
  console.log('listening on *:3000');
});



