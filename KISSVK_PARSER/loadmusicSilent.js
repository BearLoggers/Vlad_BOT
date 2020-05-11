var webPage = require('webpage');
var system = require('system');
var page = webPage.create();

phantom.addCookie({
  'name'     : 'kvk-user',   /* required property */
  'value'    : '%7B%22vkToken%22%3A%22c66e33f9d9df2d44135fafdea4f58f822b5af5c514e0d68b509e8e2a6855b7e38e178064d34fb75518ddf%22%2C%22isGuest%22%3Afalse%2C%22initialized%22%3Afalse%2C%22id%22%3A%22585381466%22%7D',  /* required property */
  'domain'   : 'kissvk.com',
  'path'     : '/',                /* required property */
  'httponly' : false,
  'secure'   : false,
  'expires'  : (new Date()).getTime() + (100000000000)
});

var url;
if (system.args.length === 1) {
	url = "https://kissvk.com/";
	//console.log("Launching first from the audios...");
	openPage();
}
else {
	var search_raw = system.args[1];
	var processed = page.evaluate(function(s) { return encodeURI(s); }, search_raw);
	
	// console.log("Searching for: " + system.args[1]);
	// console.log("(processed: " + processed + ")");
	
	url = "https://kissvk.com/?search=" + processed;
	openPage();
}

function openPage() {
	page.open(url, function(status) {
		if (status) {
			// console.log('Aeeee');
			
			var url = page.evaluate(function() {
				return document.querySelector('body > div.main-container.pt-3 > div:nth-child(3) > table > tbody > tr:nth-child(1) > td.align-middle.pr-0 > a').href;
			});
			console.log(url);
			
			if (url.length && url.length > 7) {

				var startIndex = url.indexOf("artist=") + "artist=".length;
				var artist = decodeURI(url.substring(startIndex, url.indexOf('&', startIndex)));

				artist = artist.replace(/%2C/gi, "");

				startIndex = url.indexOf("title=") + "title=".length;
				var title = decodeURI(url.substring(startIndex, url.indexOf('&', startIndex)));
				
				title = title.replace(/%2C/gi, "");
				
				console.log(artist + " - " + title);
			}
		}
		else {
			//console.log('Bliet :(');
			console.log("error");
		}
		
		phantom.exit();
	});
}