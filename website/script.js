function brightness() {
    var brightness = document.getElementById("in_bright").value;

    if(brightness > 16) {
        brightness = 16;
    } else if(brightness < 0) {
        brightness = 0;
    }

    location.href = 'update/brightness/' + brightness;
}

function refresh() {
    location.href = 'update/refrsh';
}

function id() {
    var brightness = document.getElementById("in_id").value;
    location.href = 'update/playerid/' + id;
}