var topic_prefix = "ggm-web-radio-2/";
const host = "broker.emqx.io";
const port = 8084;
const clientid = "ggm-web-radio-control-" + parseInt(Math.random() * 1000);
const mqtt_options = {
    useSSL: true,
    onSuccess: onConnect,
    onFailure: onFail
}

var pubsub;
var newborn = true;
var listIndex;
var listCache;
var sigvolt;
var statusLabel;
var sourceType;
var sourceName;
var sourceTitle;
var sourceVol;
var playlist;
var urlfreq;
var radio;
var sleepBut;
var addBut;
var renameBut;

function onLoad() {
    sigvolt = document.getElementById('sigvolt')
    statusLabel = document.getElementById('status')
    sourceType = document.getElementById('sourceType')
    sourceName = document.getElementById('sourceName')
    sourceTitle = document.getElementById('sourceTitle')
    sourceVol = document.getElementById('sourceVol')
    playlist = document.getElementById('playlist')
    urlfreq = document.getElementById('urlfreq')
    radio = document.getElementById('radio')
    sleepBut = document.getElementById('sleep')
    addBut = document.getElementById('add')
    renameBut = document.getElementById('rename')

    startConnect();
}

function sendCommand(msg) {
    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = topic_prefix + "command";
    pubsub.send(Message);
    console.log(Message.destinationName + ": " + Message.payloadString);
}

function onConnect() {
    console.log("Connected");
    pubsub.subscribe(topic_prefix + "status");
    pubsub.subscribe(topic_prefix + "list");
    statusLabel.innerHTML = "Waiting for device..."

    sendCommand('{ "status": 1 }');
}
function onFail() {
    statusLabel.innerHTML = "Failed to connect, trying again in 5 sec..."
    console.warn(statusLabel.innerHTML)
    setTimeout(startConnect, 5000);
}
function onConnectionLost(responseObject) {
    console.warn("Connection lost");
    if (responseObject != 0) {
        console.error("Error: " + responseObject.errorMessage);
    }
    statusLabel.innerHTML = "Disconnected, trying again in 5 sec..."
    console.warn(statusLabel.innerHTML)
    setTimeout(startConnect, 5000);
}
function onMessageArrived(message) {
    console.log("Message arrived: " + message.payloadString);
    if (newborn) {
        sendCommand('{ "list": 1 }');
        radio.classList.remove('hidden')
        sleepBut.classList.remove('hidden')
        newborn = false;
    }
    if (message.destinationName == (topic_prefix + "status")) {
        try {
            const obj = JSON.parse(message.payloadString);
            statusLabel.innerHTML = "Ready";
            sigvolt.innerHTML = parseFloat(obj["vlt"]) + "v<br/>RSSI: " + obj["rssi"] + "dBm";
            src = obj["src"];
            if (document.activeElement !== sourceName) renameBut.classList.add('hidden')
            sourceName.tag = 0;
            sourceName.value = obj["name"];
            sourceTitle.innerHTML = (obj["title"] ? obj["title"] : "");
            if (src % 10 == 1) {
                sourceType.innerHTML = "WEB"
                sourceVol.max = 21;
            } else {
                sourceType.innerHTML = "FM"
                sourceVol.max = 30;
            }
            if (src < 10) addBut.classList.remove('hidden')
            else addBut.classList.add('hidden')
            sourceVol.value = obj["vol"];
        } catch (e) {
            console.log(e)
            console.log(message.payloadString)
            statusLabel.innerHTML = "Data Error"
        }
    } else if (message.destinationName == (topic_prefix + "list")) {
        if (message.payloadString == "-") {
            listCache = "";
            listIndex = 0;
            playlist.classList.add('updating')
        } else if (message.payloadString == "+") {
            playlist.innerHTML = listCache;
            playlist.classList.remove('updating')
        } else {
            parts = message.payloadString.split(",");
            source = (parts[1] == "0") ? parts[2] : parts[1]
            row = ""
            if (parts[0] == "1") row += "<tr class='curr'><td>"
            else row += "<tr><td>"
            row += '<img src="up.svg" onclick="onUp(' + listIndex + ')"/><div>'
            row += listIndex
            row += '</div><img src="down.svg" onclick="onDown(' + listIndex + ')"/>'
            row += '</td><td class="wide"'
            if (parts[0] == "0") row += ' onclick="onPlay(\'' + source + '\')"'
            row += '><div>' + parts[3] + '</div><span>'
            row += (parts[1] == "0") ? (parts[2]) : (parseFloat(parts[1]) / 10)
            row += "</span></td><td>"
            row += '<img src="remove.svg" onclick="onRemove(' + listIndex + ',this)"/>'
            row += "</td></tr>"
            listCache += row
            listIndex++;
        }
    }
}

function onVolume() {
    if (sourceType.innerHTML == "WEB") {
        sendCommand('{ "webvol": ' + sourceVol.value + ' }');
    } else if (sourceType.innerHTML == "FM") {
        sendCommand('{ "fmvol": ' + sourceVol.value + ' }');
    }
}

function onPlay(source) {
    if (!source) {
        source = urlfreq.value
        freq = parseFloat(source) * 10;
        if (freq >= 870 && freq < 1080) source = freq;
    }
    sendCommand('{ "play": "' + source + '" }');
}

function onAdd() {
    sendCommand('{ "add": 1 }')
}

function onNameStart() {
    renameBut.classList.remove('hidden')
}

function onNameEnd() {
    sendCommand('{ "status": 1 }');
}

function onNameChange() {
    sourceName.tag = 1;
}

function onRename() {
    sourceName.value = sourceName.value.trim();
    if (sourceName.tag == 1 && sourceName.value.length > 0) sendCommand('{ "rename": -1, "name": "' + sourceName.value + '" }');
    //console.log(event.target)
}

function onUp(index) {
    sendCommand('{ "up": ' + index + ' }')
}

function onDown(index) {
    sendCommand('{ "down": ' + index + ' }')
}

function onRemove(index, obj) {
    if (confirm('Remove "'+obj.parentNode.previousSibling.childNodes[0].innerHTML+'"?')) sendCommand('{ "remove": ' + index + ' }')
}

function onDisplay(mode) {
    sendCommand('{ "display": ' + mode + ' }')
}

function onSleep() {
    sendCommand('{ "sleep": 1 }')
    statusLabel.innerHTML = "Going to sleep..."
    setTimeout(() => { document.location.reload() }, 2000)
}

function startConnect() {
    console.log("Attemting to connect MQTT broker...");
    pubsub = new Paho.MQTT.Client(host, port, clientid);
    pubsub.onConnectionLost = onConnectionLost;
    pubsub.onMessageArrived = onMessageArrived;
    pubsub.connect(mqtt_options);
}