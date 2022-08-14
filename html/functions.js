var fver = "1.0.1";
function gei(i){return document.getElementById(i);}
function gval(i){return gei(i).value.trim();}
function gcheck(i){return gei(i).checked;}
function sval(i,v){gei(i).value=v;}
function scheck(i){gei(i).checked=true;}
function shtml(i,v){gei(i).innerHTML=v;}
function ghtml(i){return gei(i).innerHTML;}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}
function getString(array, s, l) {
  let str = ""
  for (let i = s; i < s+l && array[i] != 0 && i < array.length; i++) {
	if (array[i] == 0 || array[i] == undefined) break;
    str += String.fromCharCode(array[i]);
  }
  return str.trim();
}
function getInt32(array, s) {
	let val = array[s++];
	val += array[s++] << 8;
	val += array[s++] << 16;
	val += array[s++] << 24;
	return val;
}
function fromString(array, s, l, val) {
  for (let i = 0; i < l; i++) {
    if (i < val.length) {
      array[s+i] = val.charCodeAt(i);
    } else {
      array[s+i] = 0;
    }
  }
}
function fromInt32(array, s, elId) {
	let val = Number(gval(elId));
	array[s++] = val & 0xFF;
	array[s++] = (val >> 8) & 0xFF;
	array[s++] = (val >> 16) & 0xFF;
	array[s++] = (val >> 24) & 0xFF;
}

function getRelays(query) {
	call("relay" + query,null, null,null,null, function (arrayBuffer) {
      if (arrayBuffer) {
		let relayObjs = {};
        let byteArray = new Uint8Array(arrayBuffer);
        let pos = 4; // pula header
		relayObjs.nodeName = getString(byteArray, pos, 15); pos += 16;
		relayObjs.relays = [];
		relayObjs.relays[0] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.relays[1] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.relays[2] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.buttons = [];
		relayObjs.buttons[0] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.buttons[1] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.buttons[2] = getString(byteArray, pos, 15); pos += 16;
		relayObjs.relayVals = [];
		relayObjs.relayVals[0] = byteArray[pos++];
		relayObjs.relayVals[1] = byteArray[pos++];
		relayObjs.relayVals[2] = byteArray[pos++];
		console.log(relayObjs);
		if (relayObjs.relayVals[0] != 255) {
			gei("relay0").checked = relayObjs.relayVals[0] == 1;
			shtml("llrelay0",relayObjs.relays[0]);
		} else {
			gei("lrelay0").style.display="none";
		}
		if (relayObjs.relayVals[1] != 255) {
			gei("relay1").checked = relayObjs.relayVals[1] == 1;
			shtml("llrelay1",relayObjs.relays[1]);
		} else {
			gei("lrelay1").style.display="none";
		}
		if (relayObjs.relayVals[2] != 255) {
			gei("relay2").checked = relayObjs.relayVals[2] == 1;
			shtml("llrelay2",relayObjs.relays[2]);
		} else {
			gei("lrelay2").style.display="none";
		}
      } else {
		alert("Erro");
		return;
      }
	}, true);
}
function getButtons() {
	call("config.dat",null,null,null, null, function (arrayBuffer) {
		let obj = parseConfig(arrayBuffer);
		for (let i = 0; i < 3; i++) {
			if (obj.buttonPins[i] != 255) {
				shtml("button"+i,obj.buttons[i]);
			} else {
				gei("button"+i).style.display="none";
			}
		}
	});
}
function pads(num) {
	return (''+num).padStart(2,'0');
}
function pad16(num) {
	return num.toString(16).padStart(2,'0');
}
function createColor(r,g,b){
	return "#"+pad16(r)+pad16(g)+pad16(b);
}
function parseColor(input){
	m = input.match(/^#([0-9a-f]{6})$/i)[1];
	if( m) {
		return [
			parseInt(m.substr(0,2),16),
			parseInt(m.substr(2,2),16),
			parseInt(m.substr(4,2),16)
		];
	}
}
function clearFields(node) {
    var inputNodes = node.querySelectorAll('INPUT');
	inputNodes.forEach(function(inputNode) {
        inputNode.value = '';
	});
}
function limitLen(el, max) {
	if (el.value.length > max) el.value = el.value.slice(0, max);
}
function onlyNums(event) {
	return event.charCode >= 48 && event.charCode <= 57;
}
function hasValue(el) {
	if (typeof el === 'string' || el instanceof String) el = gei(el);
	return el.value.trim().length > 0;
}

function changeColor(){
	let c=gval("color");
	let c1=parseColor(c);
	call("/npColor?r="+c1[0]+"&g="+c1[1]+"&b="+c1[2],null,null,null, function (resp) {
		var toast = new iqwerty.toast.Toast();
		toast.setText('Ok!').show();
	});
}
function storm(){
	call("/npStorm?i="+gval("intensity")+"&d="+gval("duration"),null,null,null, function (resp) {
		var toast = new iqwerty.toast.Toast();
		toast.setText('Ok!').show();
	});
}
function uploadFileBin(byteArray, filename, reloadUrl) {
	console.log(byteArray);
	let u8Array = Uint8Array.from(byteArray);
	console.log(new TextDecoder("ascii").decode(u8Array));
	// download local
	let dlink = gei("dblob");
	if (dlink) {
		var blob = new Blob([u8Array], {type: "image/jpg"});
		dlink.download = "blob";
		dlink.href = window.URL.createObjectURL(blob);
		dlink.dataset.downloadurl =  ['image/jpg', dlink.download, dlink.href].join(':');
	}
	// post as file upload to filename
	var formData = new FormData();
	formData.append("blob", new Blob([u8Array], {type: "image/jpg"} ),filename);
    var oReq = new XMLHttpRequest();
	oReq.onload = function(res) {
		console.log(this.statusText);
		console.log(res);
		var toast = new iqwerty.toast.Toast();
		toast.setText('Ok!').show();
		// call reload programs
		if (reloadUrl) {
			call(reloadUrl,null,null,null, function (resp) {
				console.log(filename + " reloaded");
			});
		}
	}
	oReq.onerror = function() {
		console.log(this.statusText);
	}
	oReq.open("post", "/fupload", true);
	oReq.send(formData);
}
function call(url,form,headers,post,callback,bin) {
  var oReq = new XMLHttpRequest();
  if (bin) {
  	oReq.responseType = "arraybuffer";
	oReq.onload = function(res) {
	  if(bin) {bin(this.response)}
	}
  } else {
	oReq.onload = function(res) {
	  if(callback) {callback(this.responseText)}
	}
  }
  oReq.onerror = function() {
    console.log(this.statusText);
  }
  if(post || form){
  	oReq.open("post", url, true);
  }else{
  	oReq.open("get", url, true);
  }
  if(headers){
    for (var ii=0;ii<headers.length;ii++) {
      oReq.setRequestHeader(headers[ii].name, headers[ii].value);
    }
  }
  if(post){oReq.setRequestHeader("Content-Type", "application/json");oReq.send(post);
  } else if(form){
    var elem=form.elements;var params="";var value;
    for (var i = 0; i < elem.length; i++) {
      value=null;
      if (!elem[i].disabled && elem[i].name && elem[i].name.length > 0) {
        if (elem[i].tagName == "SELECT") {
          if (elem[i].selectedIndex!=-1) value = elem[i].options[elem[i].selectedIndex].value;
        } else if (elem[i].type && (elem[i].type === 'checkbox' || elem[i].type === 'radio')) {
          if (elem[i].checked) value = elem[i].value;
        } else {
          value = elem[i].value;
        }
        if (value != null) {
          if (params.length>0)params+='&';
          params+=elem[i].name+"="+encodeURIComponent(value);
        }
      }
    }
    oReq.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    oReq.send(params);
  }else{oReq.send(null);}
}
function copyDiv(parentId, templid, num, cloneidPrefix, indexSelector, templateDatasetId) {
    let divNode = document.getElementById(templid);
	const clone = divNode.cloneNode(true);
	clone.id = cloneidPrefix + num;
	clone.dataset[templateDatasetId]=num;
    let inputNodes = clone.querySelectorAll('INPUT');
	inputNodes.forEach(function(inputNode) {
        inputNode.id = inputNode.id.substring(0, inputNode.id.length - 1) + num;
	});
    let selectNodes = clone.querySelectorAll('SELECT');
	selectNodes.forEach(function(selectNode) {
        selectNode.id = selectNode.id.substring(0, selectNode.id.length - 1) + num;
	});
    let labelNodes = clone.querySelectorAll('LABEL');
	labelNodes.forEach(function(labelNode) {
		if (labelNode.id && labelNode.id.length > 4 && labelNode.id[labelNode.id.length - 1] == '0')
			labelNode.id = labelNode.id.substring(0, labelNode.id.length - 1) + num;
	});
    let buttonNodes = clone.querySelectorAll('.clone');
	buttonNodes.forEach(function(buttonNode) {
        buttonNode.id = buttonNode.id.substring(0, buttonNode.id.length - 1) + num;
	});
	if (indexSelector) {
		let el = clone.querySelector(indexSelector);
		el.innerHTML = "" + num;
	}
	document.getElementById(parentId).appendChild(clone);
}