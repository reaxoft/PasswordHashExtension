// Connect to Native Messaging Host
function connectNativeApp(success) {
    var request = new Object();
    request.type = "connect";

    browser.runtime.sendMessage(request,
        function (response) {
            if (response.success) {
                success();
            } else {
                alert("Can't connect to native app");
            }
        });
}

// Disconnect from Native Messaging Host
function disconnectNativeApp(success) {
    var request = new Object();
    request.type = "disconnect";

    browser.runtime.sendMessage(request,
        function (response) {
            if (response.success) {
                success();
            } else {
                console.error(response.message);
            }
        });
}

var hexTable = '0123456789abcdef';
function genSalt() {
    var salt = ""
    for (var i = 0; i < 64; i++)
        salt += hexTable.charAt(Math.floor(Math.random() * hexTable.length))
    return salt;
}

function askPassword(salt, success) {
    var request = new Object();
    request.type = "AskPassword";
    request.message = salt;
    browser.runtime.sendMessage(request, success);
}


function connected() {
    document.getElementById('connect').disabled = true;
    document.getElementById('disconnect').disabled = false;
    document.getElementById('askPassword').disabled = false;
    document.getElementById('askPassword').addEventListener("click", function () {
        var salt = genSalt();
        askPassword(salt, function (hash) {
            document.getElementById("hash").value = "Salt: " + salt + "\n" + "Hash: " + hash;
        });
    });
}

function disconnected() {
    document.getElementById('connect').disabled = false;
    document.getElementById('disconnect').disabled = true;
    document.getElementById('askPassword').disabled = true;
}

document.getElementById("connect").addEventListener("click", function () {
    connectNativeApp(connected);
});
document.getElementById("disconnect").addEventListener("click", function () {
    disconnectNativeApp(disconnected);
});