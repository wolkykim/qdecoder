//--------------------------------------------------------------
//-- The qDecoder Project             http://www.qDecoder.org --
//--                                                          --
//--           Copyright (C) 2007 Seung-young Kim             --
//--------------------------------------------------------------

//
// User Define Variables
//
var Q_UPLOAD_DIALOGUE_WIDTH="380";
var Q_UPLOAD_DIALOGUE_HEIGHT="140";

var Q_UPLOAD_DRAWRATE="1000";	// set drawing interval (how often the server checks the status)
var Q_UPLOAD_TEMPLATE="qDecoder-upload/qDecoder-upload.html";	// set progress template

//
// DO NOT MODIFY BELOW UNTIL YOU UNDERSTAND WHAT IT DOES EXACTLY
//

function Q_UPLOAD(qForm) {
  var Q_UPLOAD_ACTION;
  var Q_UPLOAD_ID;

  var qBrowserVersion;

  // check field
  if(!eval(qForm.Q_UPLOAD_ID)) {
    alert("Q_UPLOAD_ID must be defined.");
    return false;
  }

  // set Q_UPLOAD_ACTION
  Q_UPLOAD_ACTION = qForm.action;

  // generate Q_UPLOAD_ID
  Q_UPLOAD_ID = (new Date()).getTime() % 1000000000;
  qForm.Q_UPLOAD_ID.value = Q_UPLOAD_ID;

  // check browser
  qBrowserVersion = navigator.appVersion;
  if (qBrowserVersion.indexOf('MSIE') != -1 && qBrowserVersion.substr(qBrowserVersion.indexOf('MSIE')+5,1) > 4) {
    winstyle = "dialogWidth="+Q_UPLOAD_DIALOGUE_WIDTH+"px; dialogHeight:"+Q_UPLOAD_DIALOGUE_HEIGHT+"px; center:yes; help:no; scroll:no; status:no;";
    window.showModelessDialog(Q_UPLOAD_ACTION+"?Q_UPLOAD_ID="+Q_UPLOAD_ID+"&Q_UPLOAD_DRAWRATE="+Q_UPLOAD_DRAWRATE+"&Q_UPLOAD_TEMPLATE="+Q_UPLOAD_TEMPLATE, null, winstyle);
    //window.open(Q_UPLOAD_ACTION+"?Q_UPLOAD_ID="+Q_UPLOAD_ID+"&Q_UPLOAD_DRAWRATE="+Q_UPLOAD_DRAWRATE+"&Q_UPLOAD_TEMPLATE="+Q_UPLOAD_TEMPLATE, null, "width=380,height=110,status=no,toolbar=no,menubar=no,location=no,resizable=no,scrollbars=no,copyhistory=no");
  }
  else {
    alert("Sorry, only supports Microsoft Explorer.");
    return false;
  }

  return true;
}
