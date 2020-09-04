// 2:37 17/03/19
firebase.initializeApp(firebaseConfig);

var load = null;
var key = 0;

firebase.auth().onAuthStateChanged(firebaseUser =>{
	if(firebaseUser){
		var userRoot = 'FirebaseESP_Smart_Control/' + firebaseUser.uid + '/';
		var init = firebase.database().ref(userRoot + 'init');
		var motherDevices = firebase.database().ref(userRoot + 'motherDevices');
		var count = Math.floor(Math.max(document.documentElement.clientWidth, window.innerWidth || 0)/350);
		count = count==0?1:count;	
		document.getElementById('panels').innerHTML = "";
		document.getElementById('buttons').innerHTML = "";
		
		init.once('value', snap => {
			if (snap.val() == null){
				init.set(true);
			}
		});
			
		motherDevices.once('value', snap => {	
			var panels = document.getElementById('panels');
			panels.innerHTML = "";
			panels.setAttribute('align', 'center');
			
			var empty = true;
			
			if (snap.val() != null){	
				snap.forEach(function(motherDevice){
					var macAddress = motherDevice.key;
					var motherRoot = userRoot + 'motherDevices/' + macAddress;
					if(motherDevice.val().Devices != undefined){
						var request = false;
						Object.values(motherDevice.val().Devices).forEach((deviceInfo) => {
							empty = false;
							var device = getDeviceByInfo(deviceInfo, macAddress);
							var devicePath = motherRoot + '/Devices/' + device.id;
							createDevicePanel(device, motherRoot);

							firebase.database().ref(devicePath).on('value', snap => {
								if(snap.val() == null){
									var el = document.getElementById(devicePath + "Panel");
									el.parentNode.removeChild(el);
									var elbutton = document.getElementById(devicePath + "Button");
									elbutton.parentNode.removeChild(elbutton);
									return;
								}

								var device = getDeviceByInfo(snap.val(), macAddress);
								var db = document.getElementById(devicePath + "Button");
								if(db == null){
									var el = document.getElementById(devicePath + "Panel");
									if(el != undefined) el.parentNode.removeChild(el);
									createDevicePanel(device, motherRoot);
								}else{
									if(db.getAttribute("loaded") == "true") {
										db.disabled = false;
										var timeout = db.getAttribute("timeout");
										if (timeout != null) {
											timeout = parseInt(timeout);
											clearInterval(timeout);
										}
										
										var to = setTimeout(setOffline, 5000, devicePath);
										db.setAttribute("timeout", to);
									}
									if(db.getAttribute("room") != device.room) {
										db.setAttribute("room", device.room);
										refreshRoomList();
									}
								}
								
								if(db.getAttribute("loaded") == "false") {
									db.setAttribute("loaded", "true");
									return;
								}
								
								var labels = document.getElementsByClassName(devicePath + "Name");
								for(var i in labels){
									labels[i].innerHTML = device.name;
								}
								
								var status = false;
								// if(device.unit != ""){
									// status = device.mode;
									// if(status)document.getElementById(devicePath + "Mode").innerHTML = "<b>Mode: HIGH</b>";
									// else document.getElementById(devicePath + "Mode").innerHTML = "<b>Mode: LOW</b";
								// }
								
								// if(device.unit != ""){
									// status = device.control;
									// if(status)document.getElementById(devicePath + "Control").innerHTML = "<b>Control: Auto</b>";
									// else document.getElementById(devicePath + "Control").innerHTML = "<b>Control: Manual</b";
								// }
								
								var icon = device.device?"./img/poweron.png":"./img/poweroff.png";
								var icons = document.getElementsByClassName(devicePath + "Icon");
								for(var i in icons){
									icons[i].src = icon;
								}
								
								if(device.unit != ""){
									document.getElementById(devicePath + "Auto").innerHTML = "Auto at: " + device.auto/10;
									document.getElementById(devicePath + "InputAuto").value = device.auto/10;
								}
								
								if(device.unit != ""){
									document.getElementById(devicePath + "Range").innerHTML = "Range: " + device.range/10;
									document.getElementById(devicePath + "InputRange").value = device.range/10;
								}
								
								if(device.unit != ""){
									document.getElementById(devicePath + "Number").innerHTML = Math.round(device.value*10)/10;
									
									var currentdate = new Date(); 
									var datetime = timeFix(currentdate.getDate()) + "/"
										+ timeFix((currentdate.getMonth()+1))  + "/" 
										+ currentdate.getFullYear() + "\n"  
										+ timeFix(currentdate.getHours()) + ":"  
										+ timeFix(currentdate.getMinutes()) + ":" 
										+ timeFix(currentdate.getSeconds());
										
									document.getElementById(devicePath + "Date").innerHTML = datetime;
								}
								
								document.getElementById(devicePath + "Loader").classList.add("hide");
							});		
							
							if(!request) {
								request = true;
								var cmdObject = firebase.database().ref(motherRoot + "/cmd");
								cmdObject.set("C:" + device.id + ":" + getNewKey());
							}
						});
					}
				});
			}
			
			if(empty){
				var panel = document.createElement('div');
				panel.setAttribute('align', 'center');
				panel.style.paddingTop = '50px'
				panel.innerHTML = `
					<b style="color:white;font-size:40px">You don't have any connected devices.</b>
				`;	
				panels.appendChild(panel);
			}

			document.getElementById('uid').innerHTML = "Login as: " + firebaseUser.email;
			document.getElementById('uidText').value = firebaseUser.uid;
			document.getElementById("loaderPanel").classList.add("hide");
			document.getElementById("loginPanel").classList.add("hide");
			document.getElementById("panelsWrapper").classList.remove("hide");
		});
	}else {
		document.getElementById('panelsWrapper').classList.add("hide");
		document.getElementById("loginPanel").classList.remove("hide");
		document.getElementById("loaderPanel").classList.add("hide");
		accountpanel();
	}
});

getNewKey = function(){
	key = key + 1;
	return key;
}

getDeviceByInfo = function(deviceinfo, macAddress){
	var device = {};
	var params1 = deviceinfo.value.split(":");
	device.id = params1[0];
	device.value = parseFloat(params1[1]);
	device.unit = params1[2];
	device.mode = params1[3] !== "0";
	device.control = params1[4] !== "0";
	device.device = params1[5] !== "0";
	device.auto = parseFloat(params1[6]);
	device.range = parseFloat(params1[7]);
	device.mac = macAddress;
	device.name = deviceinfo.name;
	device.room = deviceinfo.room;
	
	if(device.name == undefined){
		device.name = "Device";
		firebase.database().ref('FirebaseESP_Smart_Control/' + firebase.auth().currentUser.uid + '/' + 'motherDevices/' + macAddress + '/Devices/' + device.id + '/name')
			.set(device.name);
	}
	
	if(device.room == undefined){
		device.room = "Others";
		firebase.database().ref('FirebaseESP_Smart_Control/' + firebase.auth().currentUser.uid + '/' + 'motherDevices/' + macAddress + '/Devices/' + device.id + '/room')
			.set(device.room);
	}

	return device;
}

createDevicePanel = function(device, motherRoot){
	var devicePath = motherRoot + '/Devices/' + device.id;
	var panel = document.createElement('div');
	panel.setAttribute('id', devicePath + "Panel");
	panel.style.zIndex = 1001;
	panel.classList.add("mpanel");

	panel.innerHTML = `
		<div class="appbar">
			<button id="delete" onclick="deleteDevice('${motherRoot}','${device.id}','${device.name}')" class="mdelete fa fa-trash-o"></button>
			<button id="rename" onclick="renameDevice('${motherRoot}','${device.id}')" class="mrename fa fa-pencil"></button>
			<button onclick="back()" class="mback fa fa-arrow-left"></button>
		</div>
		<div id="${devicePath}Loader" class="childloader">
			<div align="center" class="loader"></div>
		</div>
		<div class="roomwrapper2" disabled="disabled">					
			<select id="${devicePath}" class="rooms roomfix" onchange="roomChange('${motherRoot}','${device.id}')">
				<option>All</option>
				<option>Others</option>
			</select>
			<button class="fa fa-plus addroom" style="font-size:30px" onclick="addRoom('${motherRoot}','${device.id}')"></button>
		</div>
		<div id="macaddress" class="macaddress">${device.mac}</div>
		<p id="${devicePath}Name"" align="center" class="name ${devicePath}Name">${device.name}</p>
		<button id="${devicePath}Device2" class="deviceButton2" onclick="deviceChange('${motherRoot}','${device.id}')"><img class="${devicePath}Icon" src="./img/power.png"></button>
	`;	
	
	if(device.unit !==""){
		
	}
	document.getElementById('panels').appendChild(panel);
	
	var micon = document.createElement('img');
	micon.src = "./img/power.png";
	micon.classList.add("micon");
	micon.classList.add(devicePath + "Icon");

	var msetting = document.createElement('button');
	msetting.classList.add("fa");
	msetting.classList.add("fa-gear");
	msetting.classList.add("msetting");
	msetting.style.fontSize = "36px";
	msetting.style.float = "right";
	msetting.style.display = "inline-block";
	msetting.setAttribute("onclick",'openPanel("' + devicePath + 'Panel")');
	
	var mdevicename = document.createElement('button');
	mdevicename.innerHTML = device.name;
	mdevicename.classList.add("devicename");
	mdevicename.classList.add(devicePath + "Name");
	
	var mbutton = document.createElement('button');
	mbutton.id = devicePath + "Button";
	mbutton.classList.add("devicebutton");
	mbutton.setAttribute("room", device.room);
	mbutton.setAttribute("loaded", false);
	mbutton.style.display = "inline-block";
	
	mbutton.appendChild(micon);
	mbutton.appendChild(mdevicename);
	mbutton.appendChild(msetting);
	
	document.getElementById('buttons').appendChild(mbutton);
	mbutton.onclick = function(e){
		if (e.target == msetting) return;
		deviceChange(motherRoot, device.id);
	}
	refreshRoomList();
}

timeFix = function(value){
    value = String(value);
    if(value.length == 1)
        return "0" + value;
    return value;
}

setOffline = function(devicePath) {
	var icons = document.getElementsByClassName(devicePath + 'Icon');
	for(var i in icons){
		icons[i].src = './img/power.png';
	}
}

modeChange = function(motherRoot, id){
	var cmdObject = firebase.database().ref(motherRoot + "/cmd");
	cmdObject.set("M:" + id + ":" + getNewKey());
	document.getElementById(motherRoot + "/Devices/" + id + "Loader").classList.remove("hide");
}

controlChange = function(motherRoot, id){
	var cmdObject = firebase.database().ref(motherRoot + "/cmd");
	cmdObject.set("C:" + id + ":" + getNewKey());
	document.getElementById(motherRoot + "/Devices/" + id + "Loader").classList.remove("hide");
}

deviceChange = function(motherRoot, id){
	var cmdObject = firebase.database().ref(motherRoot + "/cmd");
	cmdObject.set("D:" + id + ":" + getNewKey());
	document.getElementById(motherRoot + "/Devices/" + id + "Loader").classList.remove("hide");
	document.getElementById(motherRoot + '/Devices/' + id + "Button").disabled = true;
}

autoChange = function(motherRoot, id){
	event.preventDefault();	
	var cmdObject = firebase.database().ref(motherRoot + "/cmd");
	var auto = Math.round(parseFloat(document.getElementById(motherRoot + '/Devices/' + id + "InputAuto").value)*10);
	cmdObject.set("A:" + id + ":" + auto + ":" + getNewKey());
	document.getElementById(motherRoot + "/Devices/" + id + "Loader").classList.remove("hide");
}

rangeChange = function(motherRoot, id){
	event.preventDefault();
	var cmdObject = firebase.database().ref(motherRoot + "/cmd");
	var range = Math.round(parseFloat(document.getElementById(motherRoot + '/Devices/' + id + "InputRange").value)*10);
	cmdObject.set("R:" + id + ":" + range + ":" + getNewKey());
	document.getElementById(motherRoot + "/Devices/" + id + "Loader").classList.remove("hide");
}

roomChange = function(motherRoot, id){
	var devicePath = motherRoot + '/Devices/' + id;
	var sl = document.getElementById(motherRoot + '/Devices/' + id);
	var newRoom = sl.options[sl.selectedIndex].value;
	if(newRoom != null){
		firebase.database().ref(motherRoot + "/Devices/" + id + "/room").set(newRoom);
	}
}

addRoom = function(motherRoot, id){
	var newRoom = prompt("New room name", "New room");
	if(newRoom != null){
		firebase.database().ref(motherRoot + "/Devices/" + id + "/room").set(newRoom);
	}
}

renameDevice = function(motherRoot, id){
	var devicePath = motherRoot + '/Devices/' + id;
	var newName = prompt("Device name", document.getElementById(devicePath + "Name").innerHTML);
	if(newName != null){
		firebase.database().ref(motherRoot + "/Devices/" + id + "/name").set(newName);
	}
}

deleteDevice = function(motherRoot, id){
	var devicePath = motherRoot + '/Devices/' + id;
	if (confirm("Do you really want to delete this device: " + document.getElementById(devicePath + "Name").innerHTML + "?")) {
		firebase.database().ref(motherRoot + '/Devices/' + id).remove();
		back();
	}
}

refreshRoomList = function() {
	var list = [];
	var filter = [];
	var devices = document.getElementsByClassName("devicebutton");
	filter.push("<option>All</option>");
	for(var i=0; i < devices.length; i++) {
		var op = "<option>" + devices[i].getAttribute("room") + "</option>";
		if(!filter.includes(op) && op != "<option>Others</option>") {
			filter.push(op);
			list.push(op);
		}
	}
	filter.push("<option>Others</option>");
	list.push("<option>Others</option>");

	var roomlists = document.getElementsByClassName("rooms");
	for(var i=0; i < roomlists.length; i++) {
		if(roomlists[i].id == "filter") {
			roomlists[i].innerHTML = filter.join("");			
			selectFilter();
		}else {
			roomlists[i].innerHTML = list.join("");
			var index = list.indexOf("<option>" + document.getElementById(roomlists[i].id + "Button").getAttribute("room") + "</option>")
			roomlists[i].selectedIndex = index;
		}
	}
}

selectFilter = function() {
	var filter = document.getElementById("filter");
	var rule = filter.options[filter.selectedIndex].value;
	var devices = document.getElementsByClassName("devicebutton");
	
	for(var i=0; i < devices.length; i++) {
		if(rule == "All" || rule == devices[i].getAttribute("room")) {
			devices[i].style.display = "block";
		}else {
			devices[i].style.display = "none";
		}
	}
}

openPanel = function(id){
	var list = document.getElementsByClassName("mpanel");
	for(var i = 0; i < list.length; i++){
		if(list[i].id !== id){
			list[i].style.visibility = "hidden";
			list[i].style.opacity = "0";
		}else{
			list[i].style.visibility = "visible";
			list[i].style.opacity = "1";
		}
	}
	
	document.getElementById("buttons").classList.add("hide");
}

back = function(id){
	var list = document.getElementsByClassName("mpanel");
	for(var i = 0; i < list.length; i++){
		list[i].style.visibility = "hidden";
		list[i].style.opacity = "0";
	}
	
	document.getElementById("buttons").classList.remove("hide");
}

copyUid = function() {
	document.getElementById("uidText").select();
	document.execCommand("copy");
}

accountpanel = function() {
	document.getElementById("loginPanelForm").classList.remove("hide");
	document.getElementById("registerPanelForm").classList.add("hide");
	document.getElementById("resetPanelForm").classList.add("hide");
	document.getElementById("email").value = "";	
	document.getElementById("password").value = "";
	document.getElementById("loginStatus").innerHTML = "";
}

registerpanel = function() {
	document.getElementById("registerPanelForm").classList.remove("hide");
	document.getElementById("loginPanelForm").classList.add("hide");
	document.getElementById("resetPanelForm").classList.add("hide");
	document.getElementById("registerpassword").classList.remove("notmatch");
	document.getElementById("confirmpassword").classList.remove("notmatch");
	document.getElementById("registeremail").value = "";
	document.getElementById("registerpassword").value = "";	
	document.getElementById("confirmpassword").value = "";
	document.getElementById("registerStatus").innerHTML = "";
}

resetpanel = function(){
	document.getElementById("resetPanelForm").classList.remove("hide");
	document.getElementById("registerPanelForm").classList.add("hide");
	document.getElementById("loginPanelForm").classList.add("hide");
	document.getElementById("resetemail").value = "";
	document.getElementById("resetStatus").innerHTML = "";	
}

onLogin = function(){
	event.preventDefault();
	var email = document.getElementById("email").value;
	var password = document.getElementById("password").value;
	document.getElementById("loginPanel").classList.add("hide");
	document.getElementById("loaderPanel").classList.remove("hide");
	firebase.auth().signInWithEmailAndPassword(email, password).catch(function(error) {
		document.getElementById('loginStatus').innerHTML = 'Wrong email or password';
		document.getElementById("loginPanel").classList.remove("hide");
		document.getElementById("loaderPanel").classList.add("hide");
	});
}

onLogout = function(){
	event.preventDefault();
	back();
	firebase.auth().signOut();
	document.getElementById('loginStatus').innerHTML = '';
}

onRegister = function(){
	event.preventDefault();
	if (document.getElementById("registerpassword").value === document.getElementById("confirmpassword").value && document.getElementById("registerpassword").value !=="") {
		var email = document.getElementById("registeremail").value;
		var password = document.getElementById("registerpassword").value;
		document.getElementById("loginPanel").classList.add("hide");
		document.getElementById("loaderPanel").classList.remove("hide");
		firebase.auth().createUserWithEmailAndPassword(email, password).catch(function(error) {
			document.getElementById('registerStatus').innerHTML = error.message;
			document.getElementById("loginPanel").classList.remove("hide");
			document.getElementById("loaderPanel").classList.add("hide");
		});
	}else{
		document.getElementById("registerpassword").classList.add("notmatch");
		document.getElementById("confirmpassword").classList.add("notmatch");
	}
}

onReset = function(){
	event.preventDefault();
	var email = document.getElementById("resetemail").value;
	firebase.auth().sendPasswordResetEmail(email).then(function() {
		document.getElementById("resetStatus").style.color = "green";
		document.getElementById("resetStatus").innerHTML = "An email is sent to " + email + ". Please check your email to reset the password.";
	}).catch(function(error) {
		document.getElementById("resetStatus").style.color = "red";
		document.getElementById("resetStatus").innerHTML = error.message;
	});
}