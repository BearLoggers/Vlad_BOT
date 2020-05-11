var page = require('webpage').create();
page.open('http://google.com', function(status) {
  console.log("Status: " + status);
  if(status === "success") {
    page.render('google.png');
  }
  phantom.exit();
});