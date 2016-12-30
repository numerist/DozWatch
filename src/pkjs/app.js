var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () { callback(this.responseText); };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
	
  var url = "http://api.wunderground.com/api/" + localStorage.getItem('API_KEY') + "/conditions/q/" + pos.coords.latitude + "," + pos.coords.longitude + ".json";
	//var url = "http://api.wunderground.com/api/" + localStorage.getItem('API_KEY') + "/conditions/q/43.26,-79.9.json"; // Hamilton
  //var url = "http://api.wunderground.com/api/" + localStorage.getItem('API_KEY') + "/conditions/q/Canada/Greenside_Acres/.json";
  //var url = "http://api.wunderground.com/api/" + localStorage.getItem('API_KEY') + "/conditions/q/HI/Honolulu.json";
  //var url = "http://api.wunderground.com/api/" + localStorage.getItem('API_KEY') + "/conditions/q/43.229,-79.9911.json";
  //console.log(url);
	
  xhrRequest(url, 'GET',
    function(responseText) {
      var json = JSON.parse(responseText);
      var temperature = parseInt(10*json.current_observation.temp_f);
			var wind_kph = parseInt(10*json.current_observation.wind_kph);
			var humidity = parseInt(json.current_observation.relative_humidity.replace("%",""));
      var uv = parseInt(10*json.current_observation.UV);
			var wind_degrees = parseInt(json.current_observation.wind_degrees);
			var pressure_mb = parseInt(json.current_observation.pressure_mb);
      //var pressure_mb = parseInt(100*json.current_observation.pressure_in); from inches
			var conditions = json.current_observation.weather;
			var city = json.current_observation.display_location.city;
			var latitude = parseInt(1000000*pos.coords.latitude);
      //console.log(json.current_observation.local_time_rfc822.slice(5,25));
      //console.log(json.current_observation.observation_location.full);
			//console.log(json.current_observation.UV);
      
	console.log("sending");
			Pebble.sendAppMessage({'TEMPERATURE':temperature, 'CONDITIONS':conditions, 'CITY':city, 'HUMIDITY':humidity, 'WIND_KPH':wind_kph, 'UV':uv, 'WIND_DEGREES':wind_degrees, 'PRESSURE_MB':pressure_mb, 'LATITUDE':latitude},
        function(e) { console.log("Weather info sent to Pebble successfully!"); },
        function(e) { console.log("Error sending weather info to Pebble!"); }
      );
    }
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', 
  function(e) { 
		console.log("PebbleKit JS ready!"); 
		getWeather();
	}
);

Pebble.addEventListener('appmessage',
  function(e) { 
		if(e.payload.API_KEY) { localStorage.setItem('API_KEY',e.payload.API_KEY); }
		getWeather();
  }      
);

