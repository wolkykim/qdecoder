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

function qUpload(qForm) {
  // check field
  if(!eval(qForm.Q_UPLOAD_ID)) {
    alert("Q_UPLOAD_ID must be defined.");
    return false;
  }

  // set q_upload_action
  var q_upload_action = qForm.action;

  // generate q_upload_id
  var q_upload_id = (new Date()).getTime() % 1000000000;
  qForm.Q_UPLOAD_ID.value = q_upload_id;

  // check browser
  var qBrowserVersion = navigator.appVersion;
  winstyle = "dialogWidth="+Q_UPLOAD_DIALOGUE_WIDTH+"px; dialogHeight:"+Q_UPLOAD_DIALOGUE_HEIGHT+"px; center:yes; help:no; scroll:no; status:no;";

  if (qBrowserVersion.indexOf('MSIE') != -1 && qBrowserVersion.substr(qBrowserVersion.indexOf('MSIE')+5,1) > 4) {
    window.showModelessDialog(q_upload_action+"?Q_UPLOAD_ID="+q_upload_id+"&Q_UPLOAD_DRAWRATE="+Q_UPLOAD_DRAWRATE+"&Q_UPLOAD_TEMPLATE="+Q_UPLOAD_TEMPLATE, null, winstyle);
  } else {
    window.open(q_upload_action+"?Q_UPLOAD_ID="+q_upload_id+"&Q_UPLOAD_DRAWRATE="+Q_UPLOAD_DRAWRATE+"&Q_UPLOAD_TEMPLATE="+Q_UPLOAD_TEMPLATE, null, "width=380,height=110,status=no,toolbar=no,menubar=no,location=no,resizable=no,scrollbars=no,copyhistory=no");
  }

  return true;
}
